console.log('Loading a web page');
var page = require('webpage').create();
var url = 'https://123365.org.xycv.aert.xfv.d/';
page.open(url, function (status) {
  console.log("status callback: " + status);
  console.log("eval ret val:" + page.evaluate(function() {
    console.log("window.location.hostname = " + window.location.hostname);
    return window.location.hostname;
  }, function(retval) {
    console.log("retval: " + retval);
    phantom.exit();
  }));
});
console.log("exiting?");

setTimeout(function() { console.log("Timeout!"); }, 1000);
