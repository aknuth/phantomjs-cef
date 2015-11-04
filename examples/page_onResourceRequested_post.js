var page = new phantom.WebPage;

page.onResourceRequested = function(data, request) {
  console.log(data.method + ": " + data.url);
  if (!data.url.startsWith("http://tizag.com")) {
    request.abort();
    return;
  } else if (data.method == "POST") {
    console.log("GOT POST REQUEST!");
    console.log(JSON.stringify(data.post));
    for (var i in data.post) {
      var element = data.post[i];
      if (element.type == 1) {
        console.log(i + ": " + atob(data.post[0].bytes));
      }
    }
  }
};

  page.open('http://tizag.com/htmlT/forms.php')
    .then(function() {
      return page.evaluate(function() {
        var search = document.querySelector('form[method="post"] input[type="text"]');
        search.name = "myKey";
        search.value = "phantomjs";
        search.form.submit();
      });
    })
    .then(function() {
      return page.waitForLoaded();
    })
    .catch(function() {})
    .then(phantom.exit);

