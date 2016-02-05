var page = new phantom.WebPage;

// TODO
page.onResourceError = function(resourceError) {
  console.log("RESOURCE ERROR: " + JSON.stringify(resourceError, 1));
};

page.onResourceRequested = function(requestData, networkRequest) {
  console.log("RESOURCE REQUEST: " + JSON.stringify(requestData, 1));
};

page.onResourceReceived = function(data) {
  console.log("RESOURCE RECEIVED: " + JSON.stringify(data, 1));
};

page.open("html/loadresourceerror.html")
  .then(function() {
    console.log("Finished!");
  }, function(error) {
    console.log("Failed! " + error);
  })
  .then(phantom.exit)

