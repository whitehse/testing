const memory2 = new WebAssembly.Memory({ initial: 500 });
var malloc;
var free;
var doit;
var get_string_buffer;
var get_string_len;

/*
document.addEventListener("click", (e) => {
  const { target } = e;
  if (!target.matches("nav a")) {
    return;
  }
  e.preventDefault();
  //console.log(e);
  //hashRoute();
});
*/

function mylog (arg) {
  const stringBuffer = new Uint8Array(memory2.buffer, get_string_buffer(), get_string_len());
  let str = '';
  for (let i=0; i<stringBuffer.length; i++) {
    str += String.fromCharCode(stringBuffer[i]);
  }
  console.log(str);
}

const fileInput = document.getElementById('xlsx_upload');
fileInput.onchange = () => {
  const selectedFile = fileInput.files[0];
  console.log(selectedFile);

  const reader = new FileReader();

  reader.onloadend = evt => {
        const uint8_t_arr = new Uint8Array(evt.target.result);
        var fileSize = evt.target.result.byteLength;

        var offset = malloc(fileSize);
        let wasmSpace = new Uint8Array(memory2.buffer, offset, fileSize);
        for (var i=0; i<fileSize; i++) {
          wasmSpace[i] = uint8_t_arr[i]
        }
        console.log("File was this bytes big:" + fileSize);
        console.log("First two bytes of file (uint8_t_arr) are: " + uint8_t_arr[0] + " " + uint8_t_arr[1]);
        console.log("First two bytes of file (wasmSpace) are: " + wasmSpace[0] + " " + wasmSpace[1]);
        var ret = doit(offset, fileSize);
        console.log(ret);
    }
  reader.readAsArrayBuffer(selectedFile);
}

const locationHandler = async () => {
  var location = window.location.hash.replace("#", "");
  if (location.length == 0) {
    location = "/";
  }

  console.log(location);
  switch(location) {
    case "/":
      document.getElementById("top").style.display = "inline";
      document.getElementById("middle").style.display = "none";
      document.getElementById("bottom").style.display = "none";
      break;
    case "calculate":
      document.getElementById("top").style.display = "none";
      document.getElementById("middle").style.display = "inline";
      document.getElementById("bottom").style.display = "none";
      break;
    case "simulate":
      document.getElementById("top").style.display = "none";
      document.getElementById("middle").style.display = "none";
      document.getElementById("bottom").style.display = "inline";
      break;
  }
  //const router = urlRoutes[location] || urlRoutes[404];
  //const html = await fetch(route.template).then((response) =>
  //response.text());
  //document.getElementById("content").innerHTML = html;
  //document.title = route.title;
};

window.addEventListener("hashchange", locationHandler);

locationHandler();

let intervalID = setInterval(() => {
  rando = Math.floor(Math.random() * 100);
  document.getElementById("my40").style = "--start: 0.0; --end: 0." + rando + ";";
  //console.log("--start: 0.0; --end: 0." + rando + ";");
}, 100);

(async () => {
    const response2 = await fetch('wasm/ssmlexe.wasm');
    const bytes2 = await response2.arrayBuffer();
    const importObject2 = {
      env: {
        memory: memory2,
        imported_func: (arg) => mylog(arg),
      },
    };
    const { instance } = await WebAssembly.instantiate(bytes2, importObject2);
    //console.log(instance);
    var test = instance.exports.test;
    malloc = instance.exports.malloc;
    free = instance.exports.free;
    doit = instance.exports.doit;
    get_string_buffer = instance.exports.get_string_buffer;
    get_string_len = instance.exports.get_string_len;
    var none = test();

/*
    const memory = new WebAssembly.Memory({ initial: 2 });
    const response = await fetch('wasm/firetools.wasm');
    const bytes = await response.arrayBuffer();
    const importObject = {
      env: {
        memory: memory,
        imports: { imported_func: (arg) => console.log(arg) },
        imported_func: (arg) => console.log(arg),
      },
    };
    const { instance } = await WebAssembly.instantiate(bytes, importObject);

  //var simple_interest = instance.exports.simple_interest;
  //var accumulated = simple_interest(5000, .05, 3);
  var compound_interest = instance.exports.compound_interest;
  var accumulated = compound_interest(115000, .05, 30, 1);
  //console.log("Accumulated amount, using simple interest, on $5000 at an interest rate of 5% over three years = " + accumulated);
  console.log("Accumulated = " + accumulated);
*/
})();

