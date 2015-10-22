var phantom;
if (!phantom)
  phantom = {};
(function() {
  phantom.WebPage = function() {
    webpage = this;
    this.open = function(url, callback) {
      console.log("starting phantomjs query");
      startPhantomJsQuery({
        request: JSON.stringify({
          type: 'openWebPage',
          url: url
        }),
        persistent: false,
        onSuccess: function(response) {
          console.log(response);
          webpage.id = response;
          callback("success");
        },
        onFailure: function(errorCode, errorMessage) {
          console.log("error: " + errorCode + ": " + errorMessage);
          callback("error");
        }
      });
    };
  };
  phantom.require = function(file) {
    if (file === "webpage") {
      return { create: function() { return new phantom.WebPage; } };
    }
    native function require();
    return require(file);
  };
  phantom.exit = function() {
    native function exit();
    exit();
  };
  // can be overwritten by the user
  phantom.onError = null;
  // this is set to window.onerror by default
  phantom.propagateOnError = function(errorMessage, url, lineNumber, columnNumber, error) {
    if (typeof phantom.onError === "function") {
      // keep compatibility with old phantomjs onError handler
      phantom.onError(errorMessage, error.stack, url, lineNumber, columnNumber, error);
    } else {
      native function printError();
      if (error.stack) {
        printError("" + error.stack);
      } else {
        printError(errorMessage + " at " + url + ":" + lineNumber + ":" + columnNumber);
      }
    }
  };
})();
