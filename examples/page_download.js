var page = require('webpage').create();

page.download("https://github.com/KDAB/phantomjs-cef/archive/master.zip", "/tmp/test.zip")
    .catch(function(err) {
      console.log("FAIL! " + err);
    })
    .then(phantom.exit);
