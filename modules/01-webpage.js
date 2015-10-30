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
    this.evaluate = function(code) {
      arguments[0] = String(arguments[0]);
      return webpage.evaluateJavaScript.apply(webpage, arguments);
    };
    this.evaluateJavaScript = function(code) {
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
      args = [];
      for (var i = 1; i < arguments.length; ++i) {
        args.push(arguments[i]);
      }
      args = JSON.stringify(args);
      return phantom.internal.query({
          type: "evaluateJavaScript",
          code: code,
          args: args,
          browser: webpage.id
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
    // can be assigned by the user
    this.onError = function(error) {
      console.log(error);
    }
    this.render = function(path) {
      return phantom.internal.query({
        type: 'renderPage',
        path: path,
        width: webpage.viewportSize.width,
        height: webpage.viewportSize.height,
        browser: webpage.id
      });
    }
    this.injectJs = function(file) {
      var path = phantom.internal.findLibrary(file, webpage.libraryPath);
      if (!path) {
        return new Promise(function(resolve, reject) { reject(); });
      }
      var code = phantom.internal.readFile(path);
      if (!code) {
        return new Promise(function(resolve, reject) { reject(); });
      }
      return phantom.internal.query({
        type: "evaluateJavaScript",
        code: "function() {\n" + code + "\n}", // wrap in function so the code can be called
        url: "file://" + path, // file:// is required for proper console.log messages
        line: 0, // we prepend one line, so start at line 1
        browser: webpage.id
      }).then(function(retval) {
        if (retval && typeof(retval) === "string") {
          retval = JSON.parse(retval);
        }
        return retval;
      });
    }
    this.libraryPath = phantom.libraryPath;
    var internal = {
      // keep in sync with defaults in handler.cpp
      viewportSize: {width: 800, height: 600},
    };
    Object.defineProperty(this, "viewportSize", {
      get: function() {
        return internal.viewportSize;
      },
      set: function(value) {
        if (!value.hasOwnProperty("width") || value.width < 0 || !value.hasOwnProperty("height") || value.height < 0) {
          throw Error("Bad viewport size: " + JSON.stringify(value));
        }
        internal.viewportSize = value;
        createBrowser.then(function() {
          return phantom.internal.query({
            type: "setViewportSize",
            width: value.width,
            height: value.height,
            browser: webpage.id
          });
        });
      },
      configurable: false
    });
    // TODO: cleanup this api?
    this.sendEvent = function(type, key, char, _2, modifier) {
      if (typeof(char) === "string") {
        if (char.length != 1) {
          throw Error("Multi-char key events are not supported, input was: " + char);
        }
        char = char.charCodeAt(0);
      }
      return phantom.internal.query({
        type: "sendEvent",
        event: type,
        key: key,
        modifier: modifier,
        char: char,
        browser: webpage.id
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
        UNKNOWN: 0,
        BACK: 0x08,
        TAB: 0x09,
        CLEAR: 0x0C,
        RETURN: 0x0D,
        SHIFT: 0x10,
        CONTROL: 0x11, // CTRL key
        MENU: 0x12, // ALT key
        PAUSE: 0x13, // PAUSE key
        CAPITAL: 0x14, // CAPS LOCK key
        KANA: 0x15, // Input Method Editor (IME) Kana mode
        HANGUL: 0x15, // IME Hangul mode
        JUNJA: 0x17, // IME Junja mode
        FINAL: 0x18, // IME final mode
        HANJA: 0x19, // IME Hanja mode
        KANJI: 0x19, // IME Kanji mode
        ESCAPE: 0x1B, // ESC key
        CONVERT: 0x1C, // IME convert
        NONCONVERT: 0x1D, // IME nonconvert
        ACCEPT: 0x1E, // IME accept
        MODECHANGE: 0x1F, // IME mode change request
        SPACE: 0x20, // SPACE key
        PRIOR: 0x21, // PAGE UP key
        NEXT: 0x22, // PAGE DOWN key
        END: 0x23, // END key
        HOME: 0x24, // HOME key
        LEFT: 0x25, // LEFT ARROW key
        UP: 0x26, // UP ARROW key
        RIGHT: 0x27, // RIGHT ARROW key
        DOWN: 0x28, // DOWN ARROW key
        SELECT: 0x29, // SELECT key
        PRINT: 0x2A, // PRINT key
        EXECUTE: 0x2B, // EXECUTE key
        SNAPSHOT: 0x2C, // PRINT SCREEN key
        INSERT: 0x2D, // INS key
        DELETE: 0x2E, // DEL key
        HELP: 0x2F, // HELP key

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

        LWIN: 0x5B, // Left Windows key (Microsoft Natural keyboard)

        RWIN: 0x5C, // Right Windows key (Natural keyboard)

        APPS: 0x5D, // Applications key (Natural keyboard)

        SLEEP: 0x5F, // Computer Sleep key

        // Num pad keys
        NUMPAD0: 0x60,
        NUMPAD1: 0x61,
        NUMPAD2: 0x62,
        NUMPAD3: 0x63,
        NUMPAD4: 0x64,
        NUMPAD5: 0x65,
        NUMPAD6: 0x66,
        NUMPAD7: 0x67,
        NUMPAD8: 0x68,
        NUMPAD9: 0x69,
        MULTIPLY: 0x6A,
        ADD: 0x6B,
        SEPARATOR: 0x6C,
        SUBTRACT: 0x6D,
        DECIMAL: 0x6E,
        DIVIDE: 0x6F,

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

        NUMLOCK: 0x90,
        SCROLL: 0x91,
        LSHIFT: 0xA0,
        RSHIFT: 0xA1,
        LCONTROL: 0xA2,
        RCONTROL: 0xA3,
        LMENU: 0xA4,
        RMENU: 0xA5,

        BROWSER_BACK: 0xA6, // Windows 2000/XP: Browser Back key
        BROWSER_FORWARD: 0xA7, // Windows 2000/XP: Browser Forward key
        BROWSER_REFRESH: 0xA8, // Windows 2000/XP: Browser Refresh key
        BROWSER_STOP: 0xA9, // Windows 2000/XP: Browser Stop key
        BROWSER_SEARCH: 0xAA, // Windows 2000/XP: Browser Search key
        BROWSER_FAVORITES: 0xAB, // Windows 2000/XP: Browser Favorites key
        BROWSER_HOME: 0xAC, // Windows 2000/XP: Browser Start and Home key
        VOLUME_MUTE: 0xAD, // Windows 2000/XP: Volume Mute key
        VOLUME_DOWN: 0xAE, // Windows 2000/XP: Volume Down key
        VOLUME_UP: 0xAF, // Windows 2000/XP: Volume Up key
        MEDIA_NEXT_TRACK: 0xB0, // Windows 2000/XP: Next Track key
        MEDIA_PREV_TRACK: 0xB1, // Windows 2000/XP: Previous Track key
        MEDIA_STOP: 0xB2, // Windows 2000/XP: Stop Media key
        MEDIA_PLAY_PAUSE: 0xB3, // Windows 2000/XP: Play/Pause Media key
        MEDIA_LAUNCH_MAIL: 0xB4, // Windows 2000/XP: Start Mail key
        MEDIA_LAUNCH_MEDIA_SELECT: 0xB5, // Windows 2000/XP: Select Media key
        MEDIA_LAUNCH_APP1: 0xB6, // VK_LAUNCH_APP1 (B6) Windows 2000/XP: Start Application 1 key
        MEDIA_LAUNCH_APP2: 0xB7, // VK_LAUNCH_APP2 (B7) Windows 2000/XP: Start Application 2 key

        // VK_OEM_1 (BA) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ';:' key
        OEM_1: 0xBA,

        // Windows 2000/XP: For any country/region, the '+' key
        OEM_PLUS: 0xBB,

        // Windows 2000/XP: For any country/region, the ',' key
        OEM_COMMA: 0xBC,

        // Windows 2000/XP: For any country/region, the '-' key
        OEM_MINUS: 0xBD,

        // Windows 2000/XP: For any country/region, the '.' key
        OEM_PERIOD: 0xBE,

        // VK_OEM_2 (BF) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '/?' key
        OEM_2: 0xBF,

        // VK_OEM_3 (C0) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '`~' key
        OEM_3: 0xC0,

        // VK_OEM_4 (DB) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '[{' key
        OEM_4: 0xDB,

        // VK_OEM_5 (DC) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '\|' key
        OEM_5: 0xDC,

        // VK_OEM_6 (DD) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ']}' key
        OEM_6: 0xDD,

        // VK_OEM_7 (DE) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the 'single-quote/double-quote' key
        OEM_7: 0xDE,

        // VK_OEM_8 (DF) Used for miscellaneous characters; it can vary by keyboard.
        OEM_8: 0xDF,

        // VK_OEM_102 (E2) Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard
        OEM_102: 0xE2,

        // Windows 95/98/Me, Windows NT 4.0, Windows 2000/XP: IME PROCESS key
        PROCESSKEY: 0xE5,

        // Windows 2000/XP: Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT,SendInput, WM_KEYDOWN, and WM_KEYUP
        PACKET: 0xE7,

        ATTN: 0xF6, // Attn key
        CRSEL: 0xF7, // CrSel key
        EXSEL: 0xF8, // ExSel key
        EREOF: 0xF9, // Erase EOF key
        PLAY: 0xFA, // Play key
        ZOOM: 0xFB, // Zoom key

        NONAME: 0xFC, // Reserved for future use

        PA1: 0xFD, // VK_PA1 (FD) PA1 key

        OEM_CLEAR: 0xFE, // Clear key
      }
    };
  };
})();
