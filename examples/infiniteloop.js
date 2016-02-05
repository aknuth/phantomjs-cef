var page = new phantom.WebPage;

page.open("html/infiniteloop.html")
  .then(function() {
    console.log("Finished!");
  }, function(error) {
    console.log("Failed! " + error);
  })
  .then(phantom.exit)
