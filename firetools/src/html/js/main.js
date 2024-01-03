const memory2 = new WebAssembly.Memory({ initial: 2000 });

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

