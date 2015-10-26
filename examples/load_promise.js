var page = require('webpage').create();

var url = 'http://phantomjs.org';

page.open(url)
    .then(function() {
      console.log("opened!");
      return page.evaluate(function() {
        return window.location.hostname;
      });
    })
    .then(function(ret) {
      console.log("evaluated retval: " + ret);
    })
    .then(function() {
      console.log("SUCCESS!");
    }, function(err) {
      console.log("FAIL!" + err);
    })
    .then(phantom.exit);
