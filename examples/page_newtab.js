var page = require('webpage').create();
var tab;

// init settings before opening first site
// this is required to allow opening popups
page.settings.javascriptOpenWindows = true;

// NOTE: you are responsible of cleaning up popups that get created over time
//       otherwise you can easily run OOM
page.onPopupCreated = function(popup) {
  console.log("got popup");
  tab = popup;
};

page.open("html/newtab.html")
    .then(function() {
      return page.evaluate(function() {
        document.querySelector("a").click();
      });
    })
    .then(function() {
      return tab.waitForLoaded();
    })
    .then(function() {
      console.log("tab was loaded, it's url is: " + tab.url);
    })
    .then(function() {
      // the old page is still accessible
      return page.evaluate(function() {
        return document.querySelectorAll("a").length;
      });
    })
    .then(function(numLinks) {
      console.log("old page contains " + numLinks + " links");
      return tab.evaluate(function() {
        return document.querySelectorAll("a").length
      });
    })
    .then(function(numLinks) {
      console.log("tab contains " + numLinks + " links");
    })
    .catch(function(err) {
      console.log("FAIL! " + err);
    })
    .then(phantom.exit)

