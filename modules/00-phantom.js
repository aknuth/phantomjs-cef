/*
  Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

var phantom;

if (!phantom)
  phantom = {};

(function() {
  phantom.require = function(file) {
    if (file === "webpage") {
      return { create: function() { return new phantom.WebPage; } };
    } else if (file === "system") {
      return new phantom.System;
    }
    /// TODO:
    throw Error("require(" + file + ") is not yet implemented");
  };

  phantom.exit = function() {
    native function exit();
    exit();
  };

  phantom.injectJs = function(file) {
    var library = phantom.internal.findLibrary(file);
    if (!library) {
      return false;
    }
    var code = phantom.internal.readFile(library);
    if (!code) {
      return false;
    }
    native function executeJavaScript();
    executeJavaScript(code, library);
    return true;
  };

  // can be overwritten by the user
  phantom.onError = null;

  // internal functionality that is not supposed to be used by users of PhantomJS
  phantom.internal = {
    // this gets assigned to window.onerror by default
    propagateOnError: function(errorMessage, url, lineNumber, columnNumber, error) {
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
    },
    // callback from webpage.evaluateJavaScript which runs the script and returns the result
    handleEvaluateJavaScript: function(script, args, queryId) {
      var retval = null;
      var exception = null;
      if (!args) {
        args = [];
      }
      try {
        func = eval(script);
        retval = func.apply(null, args);
      } catch(e) {
        exception = e;
        if (e.stack) {
          exception = e.stack;
        }
      }
      // native DOM objects cannot be JSON.stringified directly
      // so instead copy the first level of data over
      // TODO: extend depth at will, but make sure we don't fall into
      // cycles
      function prepareJSONStringify(obj) {
        if (typeof(obj) !== "object")
          return obj;
        var ret = {};
        for (var k in obj) {
          ret[k] = obj[k];
        }
        return ret;
      }
      startPhantomJsQuery({
        request: JSON.stringify({
          type: 'returnEvaluateJavaScript',
          retval: JSON.stringify(prepareJSONStringify(retval)),
          exception: exception ? String(exception) : "",
          queryId: queryId
        }),
        persistent: false,
        onSuccess: function() {},
        onFailure: function() {}
      });
    },
    query: function(request) {
      return new Promise(function(resolve, reject) {
        startPhantomJsQuery({
          request: JSON.stringify(request),
          persistent: false,
          onSuccess: resolve,
          onFailure: function(errorCode, errorMessage) {
            reject(errorMessage, errorCode);
          }
        });
      });
    },
    findLibrary: function(file, libraryPath) {
      native function findLibrary();
      return findLibrary(file, libraryPath ? libraryPath : phantom.libraryPath);
    },
    readFile: function(file) {
      native function readFile();
      return readFile(file);
    },
    onScriptLoadError: function() {
      native function printError();
      printError("Failed to load script \""+ phantom.args[0] + "\". Exiting now.");
      phantom.exit();
    }
  };

  // will be initialized from code executed via PhantomJSApp::OnContextInitialized
  phantom.args = [];
  phantom.libraryPath = "";
})();
