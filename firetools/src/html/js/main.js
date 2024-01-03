const memory2 = new WebAssembly.Memory({ initial: 2000 });
var malloc;
var free;

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

const fileInput = document.getElementById('xlsx_upload');
fileInput.onchange = () => {
  const selectedFile = fileInput.files[0];
  console.log(selectedFile);

  const reader = new FileReader();

  reader.onloadend = evt => {
        //evt.target.result is an ArrayBuffer. In js, 
        //you can't do anything with an ArrayBuffer 
        //so we have to ???cast??? it to an Uint8Array
        const uint8_t_arr = new Uint8Array(evt.target.result);

        //const summands = new Uint8Array(memory2.buffer);
        const wasm_array = malloc(uint8_t_arr.length);
        wasm_array.set(uint8_t_arr);
        console.log("File was this bytes big:" + uint8_t_arr.length);
        console.log("File was this bytes big:" + wasm_array.length);

        //var dst = new ArrayBuffer(src.byteLength);
        //new Uint8Array(dst).set(new Uint8Array(src));

        //for (let i = 0; i < 10; i++) {
        //  summands[i] = i;
        //}

        //Right now, we have the file as a unit8array in javascript memory. 
        //As far as I understand, wasm can't directly access javascript memory. 
        //Which is why we need to allocate special wasm memory and then
        //copy the file from javascript memory into wasm memory so our wasm functions 
        //can work on it.

        //Retreiving our (modified) memory is also straight forward. 
        //First we get some javascript memory and then we copy the 
        //relevant chunk of the wasm memory into our javascript object.
        //const returnArr = new Uint8Array(uint8_t_arr.length);
        //If returnArr is std::vector<uint8_t>, then is probably similar to 
        //returnArr.assign(ptr, ptr + dataSize)
        //returnArr.set(window.Module.HEAPU8.subarray(uint8_t_ptr, uint8_t_ptr + uint8_t_arr.length));

        //Lastly, according to the docs, we should call ._free here.
        //Do we need to call the gc somehow?
        //window.Module._free(uint8_t_ptr);
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
      },
    };
    const { instance } = await WebAssembly.instantiate(bytes2, importObject2);
    //console.log(instance);
    var test = instance.exports.test;
    malloc = instance.exports.malloc;
    free = instance.exports.free;
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

