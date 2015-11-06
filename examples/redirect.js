var page = new phantom.WebPage;

page.onResourceRequested = function(request) {
  console.log(JSON.stringify(request));
};

page.onLoadFinished = function(status) {
  console.log("onLoadFinished: " + status);
}

// shortened URL to http://phantomjs.org
page.url = "http://bit.ly/1kzC14S";

page.waitForLoaded()
  .then(function(status) {
    console.log("finished! " + status + ": " + page.url);
  })
  .then(phantom.exit)
