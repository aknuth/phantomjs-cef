var page = require('webpage').create();

page.open("http://www.google.com")
  .then(function() {
    return page.injectJs("libs/keylogger.js");
  })
  .then(function() {
    return page.evaluate(function() {
      document.activeElement.value = "foo";
      console.log("active element value:" + document.activeElement.value);
      return 1;
    });
  })
  .then(function() {
    return page.sendEvent('keydown', page.event.key.A, 'a');
  })
  .then(function() {
    return page.sendEvent('keypress', page.event.key.A, 'a');
  })
  .then(function() {
    return page.sendEvent('keyup', page.event.key.A, 'a');
  })
  .then(function() {
    return page.sendEvent('keydown', page.event.key.B, 'B', null, page.event.modifier.shift_down);
  })
  .then(function() {
    return page.sendEvent('keypress', page.event.key.B, 'B', null, page.event.modifier.shift_down);
  })
  .then(function() {
    return page.sendEvent('keyup', page.event.key.A, 'B', null, page.event.modifier.shift_down);
  })
  .then(function() {
    return page.evaluate(function() {
      console.log("active element value:" + document.activeElement.value);
      console.assert(document.activeElement.value == "fooaB", "failed to simulate key event");
    });
  })
  .then(function() {
    return page.evaluate(function() {
      return page.sendEvent('');
      return document.querySelector("input[type=\"submit\"]").getBoundingClientRect();
    });
  })
  .then(function(rect) {
    console.log(JSON.stringify(rect));
    /// TODO: send mouse click event
  })
  .then(function() { console.log("success!"); },
        function(error) { console.log("error! " + error + "\n" + error.stack); })
  .then(phantom.exit)
