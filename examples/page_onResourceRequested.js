var page = require('webpage').create();

var url = 'http://www.useragentstring.com/';

page.onResourceRequested = function(data, request) {
  if (data.url != url) {
    // cancel all requests except the main URL
    // will prevent loads of CSS, JavaScript, images, ads, ...
    request.abort();
    return;
  }
  // change a header
  data.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; AS; rv:11.0) like Gecko";
};

page.open(url)
    .then(function() {
      return page.render("test.pdf");
    })
    .then(function() {
      console.log("SUCCESS! Have a look at test.pdf");
    }, function(err) {
      console.log("FAIL!" + err);
    })
    .then(phantom.exit);

