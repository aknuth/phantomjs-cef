console.log("injection worked! Will exit in half a second");
console.log(window.location.hostname);

setTimeout(phantom.exit, 500);
