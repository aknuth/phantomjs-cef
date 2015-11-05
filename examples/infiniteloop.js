var page = new phantom.WebPage;

page.open("file://" + page.libraryPath + "/html/infiniteloop.html")
  .then(function() {
    console.log("Finished!");
  }, function(error) {
    console.log("Failed! " + error);
  })
  .then(phantom.exit)
