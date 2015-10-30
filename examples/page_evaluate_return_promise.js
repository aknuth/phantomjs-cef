var page = require('webpage').create();

var url = 'http://phantomjs.org';

page.open(url)
    .then(function() {
      console.log("\tloaded:\t" + new Date);
      return page.evaluate(function(timeout) {
        return new Promise(function (accept) {
          console.log("waiting for " + timeout + "ms");
          setTimeout(function() {
            console.log("timed out, accepting!");
            accept();
          }, timeout);
        });
      }, 2000);
    })
    .then(function() {
      console.log("\tfinish:\t" + new Date);
    }, function(err) {
      console.log("FAIL!" + err);
    })
    .then(phantom.exit);

