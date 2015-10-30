var page = require('webpage').create();

var url = 'http://phantomjs.org';

page.viewportSize.width = 1024;
page.viewportSize.height = 786;

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
