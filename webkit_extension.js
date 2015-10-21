var phantom;
if (!phantom)
  phantom = {};
(function() {
  phantom.WebPage = function() {
    this.open = function(url, callback) {
      console.log("starting phantomjs query");
      startPhantomJsQuery({
        request: 'openWebPage',
        persistent: false,
        onSuccess: function() {
          callback("success");
        },
        onFailure: function(errorCode, errorMessage) {
          console.log(errorCode, errorMessage);
          callback("error");
        }
      });
    };
  };
  phantom.require = function(file) {
    if (file == "webpage") {
      return { create: function() { return new phantom.WebPage; } };
    }
    native function require();
    return require(file);
  };
  phantom.exit = function() {
    native function exit();
    exit();
  };
})();
