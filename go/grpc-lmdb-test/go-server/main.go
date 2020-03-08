package main

import (
//    "bytes"
//    "encoding/json"
//    "fmt"
    "log"
//    "os"
//    "runtime"
//    "time"
    "context"
    "net"
    "../sdk/go"

    "google.golang.org/grpc"
    "google.golang.org/grpc/reflection"
    "github.com/bmatsuo/lmdb-go/lmdb"
    proto "github.com/golang/protobuf/proto"
)

type server struct{}

func main() {
    env, err := lmdb.NewEnv()
    if err != nil {
        panic(err)
    }
    defer env.Close()

    // Configure and open the environment.  Most configuration must be done
    // before opening the environment.  The go documentation for each method
    // should indicate if it must be called before calling env.Open()
    err = env.SetMaxDBs(1)
    if err != nil {
        // ..
    }
    err = env.SetMapSize(1 << 30)
    if err != nil {
        // ..
    }
    err = env.Open("example", 0, 0644)
    if err != nil {
        // ..
    }

    // In any real application it is important to check for readers that were
    // never closed by their owning process, and for which the owning process
    // has exited.  See the documentation on transactions for more information.
    staleReaders, err := env.ReaderCheck()
    if err != nil {
        // ...
    }
    if staleReaders > 0 {
        log.Printf("cleared %d reader slots from dead processes", staleReaders)
    }

    // Open a database handle that will be used for the entire lifetime of this
    // application.  Because the database may not have existed before, and the
    // database may need to be created, we need to get the database handle in
    // an update transacation.
    var dbi lmdb.DBI
    err = env.Update(func(txn *lmdb.Txn) (err error) {
        dbi, err = txn.CreateDBI("example")
        return err
    })
    if err != nil {
        // ...
    }

    // The database referenced by our DBI handle is now ready for the
    // application to use.  Here the application just opens a readonly
    // transaction and reads the data stored in the "hello" key and prints its
    // value to the application's standard output.
    //err = env.View(func(txn *lmdb.Txn) (err error) {
    err = env.Update(func(txn *lmdb.Txn) (err error) {
        puterr := txn.Put(dbi, []byte("hello"), []byte("world"), 0)
        if puterr != nil {
            return puterr
        }
//        for i:= 0; i< 4500000; i++ {
//            //v, err := txn.Get(dbi, []byte("hello"))
//            _, err := txn.Get(dbi, []byte("hello"))
//            if err != nil {
//                return err
//            }
//            //fmt.Println(string(v))
//        }
        key := &service.Response {
            Result: 5,
        }

        marshalled_value, _ := proto.Marshal(key)
        //proto.Marshal(c, value, true)
        puterr = txn.Put(dbi, []byte("hi"), marshalled_value, 0)
        if puterr != nil {
            return puterr
        }

        return nil
    })
    if err != nil {
        // ...
    }

    listener, err := net.Listen("tcp", ":4040")
    if err != nil {
        panic(err)
    }

    srv := grpc.NewServer()
    service.RegisterMathServiceServer(srv, &server{})
    reflection.Register(srv)

    if e := srv.Serve(listener); e != nil {
        panic(err)
    }
}

func (s *server) Add(ctx context.Context, request *service.Request) (*service.Response, error) {
    a, b := request.GetA(), request.GetB()

    result := a + b

    return &service.Response{Result: result}, nil
}

func (s *server) Multiply(ctx context.Context, request *service.Request) (*service.Response, error) {
    a, b := request.GetA(), request.GetB()

    result := a * b

    return &service.Response{Result: result}, nil
}
