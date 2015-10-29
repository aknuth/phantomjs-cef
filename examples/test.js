console.log('Loading a web page');
var page = require('webpage').create();
var url = 'http://phantomjs.org';
page.open(url)
  .then(function () {
    console.log("page loaded!");
    return page.evaluate(function(label) {
      console.log("window.location.hostname = " + window.location.hostname);
      console.log("arg is:" + label);
      return window.location.hostname;
    }, "foo");
  })
  .then(function(retval) {
    console.log("retval: " + retval);
  })
  .catch(function(error) {
    console.log("execution failed :( " + error);
  })
  .then(phantom.exit)

setTimeout(function() { console.log("Timeout!"); }, 1000);
