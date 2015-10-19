var phantom;
if (!phantom)
  phantom = {};
(function() {
  phantom.WebPage = function() {
    this.open = function(url, callback) {
      callback(1);
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
