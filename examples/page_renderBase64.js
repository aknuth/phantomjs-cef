var page = require('webpage').create();

page.open("http://phantomjs.org")
    .then(function () {
        return page.renderBase64("PNG");
    })
    .then(function (data) {
      console.log(data);
    })
    .catch(function(error) {
        console.log('Error: ' + error);
    })
    .then(phantom.exit);

