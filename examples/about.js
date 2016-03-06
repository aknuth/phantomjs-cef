var page = require('webpage').create();
page.viewportSize = {width: 1920, height: 1200};
page.open("chrome://version")
    .then(function () {
        return phantom.wait(300);      
    })
    .then(function () {
        return page.render("about.png");
    })
    .catch(function(error) {
        console.log('Error: ' + error);
    })
    .then(phantom.exit);