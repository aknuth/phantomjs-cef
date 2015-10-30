var page = require('webpage').create();

var url = 'http://phantomjs.org';

try {
  page.viewportSize = [100, 200];
} catch(error) {
  console.log("got error, as expected: " + error);
}

page.viewportSize = {width: 1234, height: 890};
page.viewportSize.height = 900;

console.log("viewport size is now: " + JSON.stringify(page.viewportSize));

page.open(url)
    .then(function() {
      return page.evaluate(function() {
        return [window.innerWidth, window.innerHeight];
      });
    })
    .then(function(ret) {
      console.log("window size is: " + ret[0] + ", " + ret[1]);
      console.assert(ret[0] == page.viewportSize.width, "wrong width!" + ret[0]);
      console.assert(ret[1] == page.viewportSize.height, "wrong height!" + ret[1]);
    })
    .catch(function(err) {
      console.log("FAIL!" + err);
    })
    .then(phantom.exit);
