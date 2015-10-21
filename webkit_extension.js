var phantom;
if (!phantom)
  phantom = {};
(function() {
  phantom.WebPage = function() {
    webpage = this;
    this.open = function(url, callback) {
      console.log("starting phantomjs query");
      startPhantomJsQuery({
        request: JSON.stringify({
          type: 'openWebPage',
          url: url
        }),
        persistent: false,
        onSuccess: function(response) {
          console.log(response);
          webpage.id = response;
          callback("success");
        },
        onFailure: function(errorCode, errorMessage) {
          console.log("error: " + errorCode + ": " + errorMessage);
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
