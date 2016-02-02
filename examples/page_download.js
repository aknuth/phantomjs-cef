var page = require('webpage').create();

page.onDownloadUpdated = function(downloadItem) {
  console.log(JSON.stringify(downloadItem, null, 1));

  if (downloadItem.fullPath == "/tmp/test2.zip") {
    // you can cancel the download from here
    downloadItem.cancel();
  }
};

page.onBeforeDownload = function(downloadRequest) {
  // here you can set the target location for the file
  downloadRequest.setTarget("/tmp/test2.zip");
};

page.download("https://github.com/KDAB/phantomjs-cef/archive/master.zip", "/tmp/test.zip")
    .then(function(downloadItem) {
      var start = new Date(downloadItem.startTime);
      var end = new Date();
      console.log("Download of file " + downloadItem.fullPath + " completed in " + (end.getTime() - start.getTime()) / 1000 + "seconds.")
    })
    // now trigger a download by navigating to a file download
    .then(function() {
      return page.open("http://phantomjs.org/download.html");
    })
    .then(function() {
      return page.evaluate(function() {
        return document.querySelector("a[href*='.zip']").click();
      });
    })
    .then(function() {
      return page.waitForDownload();
    })
    .catch(function(err, code) {
      if (code) {
        console.log("FAIL! " + err);
      } else {
        // expected cancel
        console.log(err);
      }
    })
    .then(phantom.exit);
