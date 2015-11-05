var page = new phantom.WebPage;

page.onResourceRequested = function(request) {
  console.log(JSON.stringify(request));
};

page.onLoadFinished = function(status) {
  console.log("onLoadFinished: " + status);
}

// shortened URL to http://phantomjs.org
page.open("http://bit.ly/1kzC14S")
  .then(function(status) {
    console.log("finished! " + status);
  })
  .then(phantom.exit)
