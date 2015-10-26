var page = require('webpage').create();

var url = 'http://phantomjs.org';

page.open(url)
    .then(function() {
      return page.render("/tmp/test.pdf");
    })
    .then(function() {
      console.log("SUCCESS!");
    }, function(err) {
      console.log("FAIL! " + err);
    })
    .then(phantom.exit);
