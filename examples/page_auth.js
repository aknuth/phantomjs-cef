var page = require('webpage').create();
page.settings.userName = 'user';
page.settings.password = 'passwd';

page.open("https://httpbin.org/basic-auth/user/passwd")
    .then(function() {
      return page.evaluate(function() {
        return document.body.innerText;
      });
    })
    .then(function(response) {
      var data = JSON.parse(response);
      if (!data.authenticated) {
        throw Error("failed to authenticate :(");
      }
      console.log("authentication successful!");
    })
    .catch(function(err) {
      console.log("FAIL! " + err);
    })
    .then(phantom.exit);

