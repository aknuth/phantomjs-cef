var page = require('webpage').create();

console.log("Starting to load at: " + (new Date));

page.settings.resourceTimeout = 2000;

// the phantomjs.org website is only reachable without the www.
// so this should trigger a timeout
page.open('http://www.phantomjs.org')
    .then(function() {
        console.log("Success at " + (new Date));
    }, function (error) {
        console.log("script failed at " + (new Date) + "\n" + (error.stack ? error.stack : error));
    })
    .then(phantom.exit);
