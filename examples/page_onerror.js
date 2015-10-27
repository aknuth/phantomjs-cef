var page = require('webpage').create();

var url = 'http://phantomjs.org';

page.open(url)
    .then(function() {
      return page.evaluate(function() {
        throw Error("catch me if you can!");
      });
    })
    .then(function(ret) {
      console.log("evaluated retval: " + ret);
    })
    .then(function() {
      console.log("SUCCESS!");
    }, function(err) {
      console.log("FAIL! " + err);
    })
    .then(phantom.exit);

page.onError = function(error) {
  console.log("Error handler: " + error);
}
