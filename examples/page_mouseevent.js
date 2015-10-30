var page = require('webpage').create();

page.open("http://www.google.com")
  .then(function() {
    return page.injectJs("libs/keylogger.js");
  })
  .then(function() {
    return page.evaluate(function(value) {
      document.activeElement.value = value;
      console.log("active element value:" + document.activeElement.value);
      return document.querySelector("input[type=\"submit\"]").getBoundingClientRect();
    }, "PhantomJS");
  })
  .then(function(rect) {
    return page.sendEvent('click', rect.left + 5, rect.top + 5);
  })
  .then(function() {
    return phantom.wait(500);
  })
  .then(function() {
    return page.evaluate(function() {
      return document.querySelector("input[name=\"q\"]").getBoundingClientRect();
    });
  })
  .then(function(rect) {
    return page.sendEvent('doubleclick', rect.left + 5, rect.top + 5);
  })
  .then(function(rect) {
    return page.evaluate(function() {
      return document.getSelection().toString();
    })
  })
  .then(function(selection) {
    if (selection != "PhantomJS") {
      throw Error("unexpected selection :-/ " + selection);
    }
  })
  .then(function() { console.log("success!"); },
        function(error) { console.log("error! " + error + "\n" + error.stack); })
  .then(phantom.exit)
