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
    var createBrowser = phantom.query({type: "createBrowser"})
      .then(function(response) {
        webpage.id = parseInt(response);
        setupWebPageSignals(webpage);
      });

    this.onLoadStarted = function() {};
    this.onLoadFinished = function(status) {};
    this.open = function(url, callback) {
      createBrowser.then(function() {
        return phantom.query({type: "openWebPage", url: url, browser: webpage.id})
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
})();
