var page = require('webpage').create();

console.log(page.libraryPath);

page.libraryPath += "/libs"

page.open("http://phantomjs.org")
  .then(function() {
    console.log("injecting!");
    return page.injectJs("injectme.js");
  })
  .then(function() { console.log("success!"); },
        function(error) { console.log("fail :(\n" + error); });
