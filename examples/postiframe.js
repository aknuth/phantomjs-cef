var page = require('webpage').create();
page.viewportSize = { width: 1920, height: 1200 };
page.settings.webSecurityEnabled = false;

page.open("https://www.deutschepost.de/de/toolbar/suchergebnisse.html?_charset_=UTF-8&q=brief")
    .then(function() {
        return phantom.wait(100);
    })
    .then(function(){
        return page.sendMouseEvent('doubleclick',['#searchframe1','#searchTerm'])
    })
     .then(function(){
         return page.sendEvent('keypress', 'Requirements');
    })
    .then(function(){
        return phantom.wait(300);
    })
    .then(function(){
        return page.render("post.png");
    })
    .then(function() {
      phantom.exit();
    })
    .catch(function(err) {
      console.log("FAIL! " + err);
      phantom.exit();
    })