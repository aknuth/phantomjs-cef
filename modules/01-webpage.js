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
  phantom.WebPage = function() {
    var webpage = this;
    var internal = {
      viewportSize: {width: 800, height: 600},
      zoomFactor: 1.,
      createBrowser: null,
      id: null,
      beforeResourceLoad: function(request, requestId) {
        if (typeof(webpage.settings.userAgent) === "string") {
          request.headers["User-Agent"] = webpage.settings.userAgent;
        }
        // TODO request.time
        var allow = true;
        var networkRequest = {
          abort: function() {
            allow = false;
          },
          changeUrl: function(url) {
            request.url = url;
          },
          setHeader: function(key, value) {
            request.headers[key] = value;
          }
        };
        webpage.onResourceRequested(request, networkRequest);
        if (internal.signalWaiters.onResourceRequested) {
          var waiter = internal.signalWaiters.onResourceRequested;
          delete internal.signalWaiters.onResourceRequested;
          waiter(request, networkRequest);
        }
        phantom.internal.query({
          type: "beforeResourceLoadResponse",
          requestId: requestId,
          request: request,
          allow: allow
        });
      },
      signalWaiters: {}
    };
    function verifyBrowserCreated() {
      if (!internal.id) {
        throw new Error("No page has been loaded. The function " + verifyBrowserCreated.caller + " must be called after the first page load event.");
      }
    }
    function createBrowser() {
      if (!internal.createBrowser) {
        internal.createBrowser = phantom.internal.query({
          type: "createBrowser",
          settings: webpage.settings
        })
        .then(function(response) {
          internal.id = parseInt(response);
          // send current values of some properties
          webpage.viewportSize = internal.viewportSize;
          webpage.zoomFactor = internal.zoomFactor;
          startPhantomJsQuery({
            request: JSON.stringify({
              type: 'webPageSignals',
              browser: internal.id
            }),
            persistent: true,
            onSuccess: function(response) {
              var response = JSON.parse(response);
              var target = response.internal ? internal : webpage;
              target[response.signal].apply(webpage, response.args);
              if (!response.internal) {
                var waiter = internal.signalWaiters[response.signal];
                if (waiter) {
                  delete internal.signalWaiters[response.signal];
                  waiter.apply(webpage, response.args);
                }
              }
            },
            onFailure: function() {}
          });
        });
      }
      return internal.createBrowser;
    }

    this.onConsoleMessage = function(message, source, line) {
      native function printError();
      printError(source + ":" + line + ": " + message);
    };
    this.onLoadStarted = function() {};
    this.onLoadFinished = function(status) {};
    this.onPaint = function() {};
    this.onResourceRequested = function(requestData, networkRequest) {};
    this.onResourceReceived = function(response) {};
    // TODO: onResourceTimeout
    this.waitForSignal = function(signal) {
      return new Promise(function(resolve) {
        internal.signalWaiters[signal] = resolve;
      });
    };
    this.open = function(url, callback) {
      var ret = createBrowser().then(function() {
        return phantom.internal.query({
          type: "openWebPage",
          url: url,
          browser: internal.id})
      });
      // TODO: this is just a workaround as it won't catch timeouts
      //       for embedded resources such as images or scripts.
      //       http://www.magpcss.org/ceforum/viewtopic.php?f=6&t=13080&p=28385#p28385
      if (webpage.settings.resourceTimeout > 0) {
        ret = Promise.race([ret,
            phantom.wait(webpage.settings.resourceTimeout).then(function() {
              webpage.stop();
              return webpage.waitForLoaded();
            })]);
      }
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
    this.waitForLoaded = function() {
      verifyBrowserCreated();
      return phantom.internal.query({
        type: "waitForLoaded",
        browser: internal.id
      });
    };
    this.waitForDomElement = function(selector, pollInterval, maxPollAttempts) {
      if (!pollInterval) {
        pollInterval = 100;
      }
      if (!maxPollAttempts) {
        maxPollAttempts = 50;
      }
      return new Promise(function(accept, reject) {
        var pollAttempts = 0;
        function pollElement() {
          webpage.evaluate(function(selector) {
            var element = document.querySelector(selector);
            return element ? element : null;
          }, selector)
          .then(function(element) {
            if (element) {
              accept(element);
            } else if (++pollAttempts > maxPollAttempts) {
              reject(Error("Timeout while waiting for DOM element: " + selector));
            } else {
              setTimeout(pollElement, pollInterval);
            }
          });
        }
        pollElement();
      });
    };
    this.stop = function() {
      verifyBrowserCreated();
      return phantom.internal.query({type: "stopWebPage", browser: internal.id});
    };
    this.close = function() {
      if (internal.id === null) {
        return;
      }
      startPhantomJsQuery({
        request: JSON.stringify({
          type: 'closeWebPage',
          browser: internal.id
        }),
        persistent: false,
        onSuccess: function() {},
        onFailure: function() {}
      });
      internal.id = null;
    };
    this.evaluateJavaScript = function(code) {
      verifyBrowserCreated();
      /*
       * this is pretty convoluted due to the multi-process architecture
       *
       * we send this query to the handler, i.e. browser process
       * this then finds the browser for the internal.id and executes
       * the script there via phantom.internal.handleEvaluateJavaScript.
       * this in turn sends the return value or exception back to the handler
       * which then triggers the callback for this query...
       *
       * i.e.: four IPC hops for a single evaluateJavaScript call :(
       */
      args = [];
      for (var i = 1; i < arguments.length; ++i) {
        args.push(arguments[i]);
      }
      args = JSON.stringify(args);
      return phantom.internal.query({
          type: "evaluateJavaScript",
          code: String(code),
          args: args,
          browser: internal.id
      }).then(function(retval) {
        if (retval && typeof(retval) === "string") {
          retval = JSON.parse(retval);
        }
        return retval;
      }, function(error) {
        if (typeof(webpage.onError) === "function") {
          webpage.onError.apply(webpage, arguments);
        }
        // rethrow so that any .then continuation can catch this in an error handler
        throw error;
      });
    };
    this.evaluate = this.evaluateJavaScript;
    // can be assigned by the user
    this.onError = function(error) {
      console.log(error);
    };
    this.render = function(path) {
      verifyBrowserCreated();
      if (path.endsWith("pdf")) {
        return phantom.internal.query({
          type: 'printPdf',
          path: path,
          paperSize: webpage.paperSize,
          browser: internal.id
        });
      } else {
        return phantom.internal.query({
          type: 'renderImage',
          path: path,
          clipRect: webpage.clipRect,
          browser: internal.id
        });
      }
    };
    this.renderBase64 = function(format) {
      verifyBrowserCreated();
      return phantom.internal.query({
        type: 'renderImage',
        format: format,
        clipRect: webpage.clipRect,
        browser: internal.id
      });
    };
    this.injectJs = function(file) {
      verifyBrowserCreated();
      var path = phantom.internal.findLibrary(file, webpage.libraryPath);
      if (!path) {
        return new Promise(function(resolve, reject) { reject(Error("Could not find file to inject: " + file)); });
      }
      var code = phantom.internal.readFile(path);
      if (!code) {
        return new Promise(function(resolve, reject) { reject(Error("Could not read file to inject: " + path)); });
      }
      return phantom.internal.query({
        type: "evaluateJavaScript",
        code: "function() {\n" + code + "\n}", // wrap in function so the code can be called
        url: "file://" + path, // file:// is required for proper console.log messages
        line: 0, // we prepend one line, so start at line 1
        browser: internal.id
      }).then(function(retval) {
        if (retval && typeof(retval) === "string") {
          retval = JSON.parse(retval);
        }
        return retval;
      });
    };
    this.libraryPath = phantom.libraryPath;
    this.paperSize = {
      format: "A4",
      margin: "default",
      orientation: "portrait"
    };
    this.clipRect = {
      top: 0,
      left: 0,
      width: -1,
      height: -1,
    };
    this.settings = {
      javascriptEnabled: true,
      loadImages: true,
      webSecurityEnabled: true,
      localToRemoteUrlAccessEnabled: false,
      javascriptOpenWindows: false,
      javascriptCloseWindows: false,
      userAgent: null,
      userName: null,
      password: null,
      resourceTimeout: 10 * 1000, // ms timeout
    };
    function addProperty(name, object) {
      Object.defineProperty(object, name, {
        get: function() {
          return internal[name];
        },
        set: function(value) {
          internal[name] = value;
          if (!internal.createBrowser) {
            // delay until page gets created
            return;
          }
          internal.createBrowser.then(function() {
            return phantom.internal.query({
              type: "setProperty",
              name: name,
              value: value,
              browser: internal.id
            });
          });
        },
        configurable: false
      });
    }
    addProperty("viewportSize", webpage);
    addProperty("zoomFactor", webpage);
    // TODO: cleanup this api?
    //       i.e. a key event and a mouse event function
    //       or take an object of args
    this.sendEvent = function(type, arg1, arg2, arg3, modifier) {
      verifyBrowserCreated();
      return phantom.internal.query({
        type: "sendEvent",
        event: type,
        arg1: arg1,
        arg2: arg2,
        arg3: arg3,
        modifier: modifier,
        browser: internal.id
      });
    };
    this.event = {
      modifier: {
        none: 0,
        caps_lock_on: 1 << 0,
        shift_down: 1 << 1,
        control_down: 1 << 2,
        alt_down: 1 << 3,
        left_mouse_button: 1 << 4,
        middle_mouse_button: 1 << 5,
        right_mouse_button: 1 << 6,
        command_down: 1 << 7,
        num_lock_on: 1 << 8,
        is_key_pad: 1 << 9,
        is_left: 1 << 10,
        is_right: 1 << 11,
      },
      key: {
        Back: 0x08,
        Tab: 0x09,
        Clear: 0x0C,
        Return: 0x0D,
        Shift: 0x10,
        Control: 0x11, // CTRL key
        Menu: 0x12, // ALT key
        Pause: 0x13, // PAUSE key
        Capital: 0x14, // CAPS LOCK key
        Kana: 0x15, // Input Method Editor (IME) Kana mode
        Hangul: 0x15, // IME Hangul mode
        Junja: 0x17, // IME Junja mode
        Final: 0x18, // IME final mode
        Hanja: 0x19, // IME Hanja mode
        Kanji: 0x19, // IME Kanji mode
        Escape: 0x1B, // ESC key
        Convert: 0x1C, // IME convert
        NonConvert: 0x1D, // IME nonconvert
        Accept: 0x1E, // IME accept
        ModeChange: 0x1F, // IME mode change request
        Space: 0x20, // SPACE key
        Prior: 0x21, // PAGE UP key
        Next: 0x22, // PAGE DOWN key
        End: 0x23, // END key
        Home: 0x24, // HOME key
        Left: 0x25, // LEFT ARROW key
        Up: 0x26, // UP ARROW key
        Right: 0x27, // RIGHT ARROW key
        Down: 0x28, // DOWN ARROW key
        Select: 0x29, // SELECT key
        Print: 0x2A, // PRINT key
        Execute: 0x2B, // EXECUTE key
        Snapshot: 0x2C, // PRINT SCREEN key
        Insert: 0x2D, // INS key
        Delete: 0x2E, // DEL key
        Help: 0x2F, // HELP key

        0: 0x30,
        1: 0x31,
        2: 0x32,
        3: 0x33,
        4: 0x34,
        5: 0x35,
        6: 0x36,
        7: 0x37,
        8: 0x38,
        9: 0x39,
        A: 0x41,
        B: 0x42,
        C: 0x43,
        D: 0x44,
        E: 0x45,
        F: 0x46,
        G: 0x47,
        H: 0x48,
        I: 0x49,
        J: 0x4A,
        K: 0x4B,
        L: 0x4C,
        M: 0x4D,
        N: 0x4E,
        O: 0x4F,
        P: 0x50,
        Q: 0x51,
        R: 0x52,
        S: 0x53,
        T: 0x54,
        U: 0x55,
        V: 0x56,
        W: 0x57,
        X: 0x58,
        Y: 0x59,
        Z: 0x5A,

        Lwin: 0x5B, // Left Windows key (Microsoft Natural keyboard)

        Rwin: 0x5C, // Right Windows key (Natural keyboard)

        Apps: 0x5D, // Applications key (Natural keyboard)

        Sleep: 0x5F, // Computer Sleep key

        // Num pad keys
        Numpad0: 0x60,
        Numpad1: 0x61,
        Numpad2: 0x62,
        Numpad3: 0x63,
        Numpad4: 0x64,
        Numpad5: 0x65,
        Numpad6: 0x66,
        Numpad7: 0x67,
        Numpad8: 0x68,
        Numpad9: 0x69,
        Multiply: 0x6A,
        Add: 0x6b,
        Separator: 0x6C,
        Subtract: 0x6D,
        Decimal: 0x6E,
        Divide: 0x6f,

        F1: 0x70,
        F2: 0x71,
        F3: 0x72,
        F4: 0x73,
        F5: 0x74,
        F6: 0x75,
        F7: 0x76,
        F8: 0x77,
        F9: 0x78,
        F10: 0x79,
        F11: 0x7A,
        F12: 0x7B,
        F13: 0x7C,
        F14: 0x7D,
        F15: 0x7E,
        F16: 0x7F,
        F17: 0x80,
        F18: 0x81,
        F19: 0x82,
        F20: 0x83,
        F21: 0x84,
        F22: 0x85,
        F23: 0x86,
        F24: 0x87,

        Numlock: 0x90,
        Scroll: 0x91,
        Lshift: 0xA0,
        Rshift: 0xA1,
        Lcontrol: 0xA2,
        Rcontrol: 0xA3,
        Lmenu: 0xA4,
        Rmenu: 0xA5,

        Browser_Back: 0xA6, // Windows 2000/XP: Browser Back key
        Browser_Forward: 0xA7, // Windows 2000/XP: Browser Forward key
        Browser_Refresh: 0xA8, // Windows 2000/XP: Browser Refresh key
        Browser_Stop: 0xA9, // Windows 2000/XP: Browser Stop key
        Browser_Search: 0xAA, // Windows 2000/XP: Browser Search key
        Browser_Favorites: 0xAB, // Windows 2000/XP: Browser Favorites key
        Browser_Home: 0xAC, // Windows 2000/XP: Browser Start and Home key
        Volume_Mute: 0xAD, // Windows 2000/XP: Volume Mute key
        Volume_Down: 0xAE, // Windows 2000/XP: Volume Down key
        Volume_Up: 0xAF, // Windows 2000/XP: Volume Up key
        Media_Next_Track: 0xB0, // Windows 2000/XP: Next Track key
        Media_Prev_Track: 0xB1, // Windows 2000/XP: Previous Track key
        Media_Stop: 0xB2, // Windows 2000/XP: Stop Media key
        Media_Play_Pause: 0xB3, // Windows 2000/XP: Play/Pause Media key
        Media_Launch_Mail: 0xB4, // Windows 2000/XP: Start Mail key
        Media_Launch_Media_Select: 0xB5, // Windows 2000/XP: Select Media key
        Media_Launch_App1: 0xB6, // VK_LAUNCH_APP1 (B6) Windows 2000/XP: Start Application 1 key
        Media_Launch_App2: 0xB7, // VK_LAUNCH_APP2 (B7) Windows 2000/XP: Start Application 2 key

        Play: 0xFA, // Play key
        Zoom: 0xFB, // Zoom key
      }
    };
  };
})();
