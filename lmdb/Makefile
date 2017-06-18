all:
	gcc -lh2o -luv -lssl -lcrypto -llmdb -o simple simple.c
	gcc -llmdb -o populate-boguscoin-values populate-boguscoin-values.c
	if [ ! -d "boguscoin_db" ]; then mkdir boguscoin_db; fi

clean:
	rm simple
	rm populate-boguscoin-values
