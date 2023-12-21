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
    case "calculators":
      document.getElementById("top").style.display = "none";
      document.getElementById("middle").style.display = "inline";
      document.getElementById("bottom").style.display = "none";
      break;
    case "simulations":
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
