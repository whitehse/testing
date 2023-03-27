<main>
  <script src="//cdn.rawgit.com/dcodeIO/protobuf.js/6.10.2/dist/protobuf.min.js"></script>
  This is the websocket page.
</main>

<style>
  @tailwind utilities;
  @tailwind components;
  @tailwind base;
</style>

<script>
const socket = new WebSocket('ws://localhost:8001/');
socket.binaryType = 'arraybuffer';

socket.addEventListener("open", function (event) {
    console.log("Connection Opened, sending message");
    socket.send('{"message": "HelloWorld!"}');
});

socket.addEventListener("error", function(err) {
    console.log("error: ", err);
});

socket.addEventListener("close", function() {
    console.log("close");
});

socket.addEventListener('message', function (event) {
  msg = event.data
  buffer = Uint8Array.from(atob(msg), c => c.charCodeAt(0))

  protobuf.load("/js/mymessage.proto", function(err, root) {

    var MyMessage = root.lookupType("MyMessage");

    var message = MyMessage.decode(buffer);

    console.log(message)   
  });
});
</script>
