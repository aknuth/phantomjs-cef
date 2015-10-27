var page = require('webpage').create();

var url = 'http://phantomjs.org';

page.open(url)
    .then(function() {
      console.log("SUCCESS!");
    }, function(err) {
      console.log("FAIL!" + err);
    })
    .then(phantom.exit);

page.onLoadStarted = page.stop;
