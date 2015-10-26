var phantom;

if (!phantom)
  phantom = {};

(function() {
  function wrapPhantomQuery(request) {
    return new Promise(function(resolve, reject) {
      startPhantomJsQuery({
        request: JSON.stringify(request),
        persistent: false,
        onSuccess: resolve,
        onFailure: reject
      });
    });
  }

  function setupWebPageSignals(webpage) {
    startPhantomJsQuery({
      request: JSON.stringify({
        type: 'webPageSignals',
        browser: webpage.id
      }),
      persistent: true,
      onSuccess: function(response) {
        var response = JSON.parse(response);
        webpage[response.signal].apply(webpage, response.args);
      },
      onFailure: function() {}
    });
  }

  phantom.WebPage = function() {
    var webpage = this;
    var createBrowser = wrapPhantomQuery({type: "createBrowser"})
      .then(function(response) {
        webpage.id = parseInt(response);
        setupWebPageSignals(webpage);
      });

    this.onLoadStarted = function() {};
    this.onLoadFinished = function(status) {};
    this.open = function(url, callback) {
      createBrowser.then(function() {
        return wrapPhantomQuery({type: "openWebPage", url: url, browser: webpage.id})
      }).then(function() {
        if (typeof(callback) === "function") {
          callback("success");
        }
      }, function() {
        if (typeof(callback) === "function") {
          callback("fail");
        }
      });
    };
    this.close = function() {
      if (webpage.id === null) {
        return;
      }
      startPhantomJsQuery({
        request: JSON.stringify({
          type: 'closeWebPage',
          browser: webpage.id
        }),
        persistent: false,
        onSuccess: function() {},
        onFailure: function() {}
      });
      webpage.id = null;
    }
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
