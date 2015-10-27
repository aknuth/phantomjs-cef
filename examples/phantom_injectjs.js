console.log(phantom.libraryPath)
phantom.libraryPath += "/libs"

var worked = phantom.injectJs("injectme.js");

console.log("did it work? " + worked);
