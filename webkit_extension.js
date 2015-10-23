var phantom;
if (!phantom)
  phantom = {};
(function() {
  phantom.WebPage = function() {
    var webpage = this;
    this.open = function(url, callback) {
      startPhantomJsQuery({
        request: JSON.stringify({
          type: 'openWebPage',
          url: url
        }),
        persistent: false,
        onSuccess: function(response) {
          webpage.id = parseInt(response);
          callback("success");
        },
        onFailure: function(errorCode, errorMessage) {
          callback("error");
        }
      });
    };
    this.evaluate = function(script, callback, errorCallback) {
      webpage.evaluateJavaScript(String(script), callback, errorCallback);
    };
    this.evaluateJavaScript = function(script, callback, errorCallback) {
      if (webpage.id === null) {
        console.log("Cannot evaluate JavaScript code before WebPage has opened.");
        return;
      }
      /*
       * this is pretty convoluted due to the multi-process architecture
       *
       * we send this query to the handler, i.e. browser process
       * this then finds the browser for the webpage.id and executes
       * the script there via phantom.handleEvaluateJavaScript.
       * this in turn sends the return value or exception back to the handler
       * which then triggers the callback for this query...
       *
       * i.e.: four IPC hops for a single evaluateJavaScript call :(
       */
      startPhantomJsQuery({
        request: JSON.stringify({
          type: 'evaluateJavaScript',
          script: script,
          browser: webpage.id
        }),
        persistent: false,
        onSuccess: function(response) {
          if (typeof(callback) === "function") {
            callback(response);
          }
        },
        onFailure: function(errorCode, errorMessage) {
          if (typeof(errorCallback) === "function") {
            errorCallback(errorCode, errorMessage);
          } else {
            console.log("evaluateJavaScript failure: " + errorCode + ": " + errorMessage);
          }
        }
      });
    };
    this.id = null;
  };
  phantom.require = function(file) {
    if (file === "webpage") {
      return { create: function() { return new phantom.WebPage; } };
    }
    /// TODO:
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
        printError(String(error.stack));
      } else {
        printError(errorMessage + " at " + url + ":" + lineNumber + ":" + columnNumber);
      }
    }
  };
  phantom.handleEvaluateJavaScript = function(script, queryId) {
    var retval = null;
    var exception = null;
    try {
      func = eval(script);
      retval = func();
    } catch(e) {
      exception = e;
      if (e.stack) {
        exception = e.stack;
      }
    }
    startPhantomJsQuery({
      request: JSON.stringify({
        type: 'returnEvaluateJavaScript',
        retval: JSON.stringify(retval),
        exception: exception ? String(exception) : "",
        queryId: queryId
      }),
      persistent: false,
      onSuccess: function() {},
      onFailure: function() {}
    });
  };
})();
