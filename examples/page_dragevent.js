var page = require('webpage').create();

var dragRect;
var dropRect;

page.onConsoleMessage = function(message) {
  console.log(message);
}

page.open("file://" + page.libraryPath + "/html/dragdrop.html")
  .then(function() {
    return page.injectJs("libs/keylogger.js");
  })
  .then(function() {
    return page.evaluate(function() {
      var ret = [document.querySelector("#dragme").getBoundingClientRect(),
              document.querySelector("#dropme").getBoundingClientRect()];
//       console.log(JSON.stringify(ret[0].top));
      return ret;
    });
  })
  .then(function(rects) {
    dragRect = rects[0];
    dropRect = rects[1];
    return page.sendEvent('mousedown', dragRect.left + 5, dragRect.top + 5);
  })
  .then(function() {
    return page.sendEvent('mousemove', dropRect.left + 5, dropRect.top + 5);
  })
  .then(function() {
    return page.sendEvent('mouseup', dropRect.left + 5, dropRect.top + 5);
  })
  .then(function() {
    return phantom.wait(500);
  })
  .then(function() {
    return page.render("dragdrop.png");
  })
  .then(function() { console.log("success!"); },
        function(error) { console.log("error! " + error + "\n" + error.stack); })
  .then(phantom.exit)

