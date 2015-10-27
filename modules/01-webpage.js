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

(function() {
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
    this.id = null;
    var createBrowser = phantom.internal.query({type: "createBrowser"})
      .then(function(response) {
        webpage.id = parseInt(response);
        setupWebPageSignals(webpage);
      });

    this.onLoadStarted = function() {};
    this.onLoadFinished = function(status) {};
    this.open = function(url, callback) {
      var ret = createBrowser.then(function() {
        return phantom.internal.query({type: "openWebPage", url: url, browser: webpage.id})
      });
      if (typeof(callback) === "function") {
        // backwards compatibility when callback is given
        return ret.then(function() {
          if (typeof(callback) === "function") {
            callback("success");
          }
        }, function() {
          if (typeof(callback) === "function") {
            callback("fail");
          }
        });
      }
      // otherwise just return the promise as-is
      return ret;
    };
    this.stop = function() {
      return createBrowser.then(function() {
        return phantom.internal.query({type: "stopWebPage", browser: webpage.id});
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
    };
    this.evaluate = function(script, successCallback, errorCallback) {
      return webpage.evaluateJavaScript(String(script), successCallback, errorCallback);
    };
    this.evaluateJavaScript = function(script, successCallback, errorCallback) {
      /*
       * this is pretty convoluted due to the multi-process architecture
       *
       * we send this query to the handler, i.e. browser process
       * this then finds the browser for the webpage.id and executes
       * the script there via phantom.internal.handleEvaluateJavaScript.
       * this in turn sends the return value or exception back to the handler
       * which then triggers the callback for this query...
       *
       * i.e.: four IPC hops for a single evaluateJavaScript call :(
       */
      return phantom.internal.query({
          type: 'evaluateJavaScript',
          script: script,
          browser: webpage.id
      }).then(successCallback, function(error) {
        if (typeof(webpage.onError) === "function") {
          webpage.onError.apply(webpage, arguments);
        }
        if (typeof(errorCallback) === "function") {
          errorCallback.apply(null, arguments);
        }
        // rethrow so that any .then continuation can catch this in an error handler
        throw error;
      });
    };
    // can be assigned by the user
    this.onError = function(error) {
      console.log(error);
    }
    this.render = function(path) {
      return phantom.internal.query({
        type: 'renderPage',
        path: path,
        browser: webpage.id
      });
    }
  };
})();
