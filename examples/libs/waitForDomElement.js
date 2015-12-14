window.waitForDomElement = function(selector, pollInterval, maxTimeout) {
  console.log('window.location.href:'+window.location.href);
  var element = document.querySelector(selector);
  if (element) {
    return element;
  }

  return new Promise(function(accept, reject) {
    var checkExist = setInterval(function() {
      var element = document.querySelector(selector);
      if (element) {
          clearInterval(checkExist);
          accept(element);
      }
    }, pollInterval ? pollInterval : 5);
    setTimeout(function() {
      clearInterval(checkExist);
      reject(Error("Timeout while waiting for DOM element: " + selector));
    }, maxTimeout ? maxTimeout : 5000);
  });
};
