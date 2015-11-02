var page = require('webpage').create();
page.settings.userAgent = 'Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; AS; rv:11.0) like Gecko';

page.open("http://www.useragentstring.com/")
    .then(function() {
      return page.render("useragent.pdf");
    })
    .catch(function(err) {
      console.log("FAIL!" + err);
    })
    .then(phantom.exit);
