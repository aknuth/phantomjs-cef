var page = require('webpage').create();

page.open("http://www.google.com")
  .then(function() {
    return page.injectJs("libs/keylogger.js");
  })
  .then(function() {
    return page.evaluate(function(value) {
      document.activeElement.value = value;
      console.log("active element value:" + document.activeElement.value);
    }, "foo");
  })
  .then(function() {
    return page.sendEvent('keydown', 13);
  })
  .then(function() {
    return page.sendEvent('keypress', 'a');
  })
  .then(function() {
    return page.sendEvent('keyup', 'a');
  })
  .then(function() {
    return page.sendEvent('keydown', 'B', null, null, page.event.modifier.shift_down);
  })
  .then(function() {
    return page.sendEvent('keypress', 'B', null, null, page.event.modifier.shift_down);
  })
  .then(function() {
    return page.sendEvent('keyup', 'B', null, null, page.event.modifier.shift_down);
  })
  .then(function() {
    return page.sendEvent('keypress', "PhantomJS");
  })
  .then(function() {
    return page.evaluate(function(value) {
      console.log("active element value:" + document.activeElement.value);
      console.assert(document.activeElement.value == value, "failed to simulate key event");
    }, "fooaBPhantomJS");
  })
  .then(function() { console.log("success!"); },
        function(error) { console.log("error! " + error + "\n" + error.stack); })
  .then(phantom.exit)
