var page = require('webpage').create();

page.viewportSize = { width: 1280, height: 1024 };

function sendEvent(type, selector) {
    return page.evaluate(function(selector){
        var element = document.querySelector(selector);
        if (!element){
            throw Error("no element found with selector:" + selector);
        }
        return element.getBoundingClientRect();
    }, selector)
    .then(function(offset) {
        return page.sendEvent(type, offset.left + 5, offset.top + 5);
    })
}

page.open('http://www.tuifly.com/de/index.html')
    .then(function() {
      return page.injectJs("libs/waitForDomElement.js");
    })
    .then(function() {
        return sendEvent('click', 'label[data-oneway=true]');
    })
    .then(function() {
        return sendEvent('click', '.js-airport-input');
    })
    .then(function() {
        return page.sendEvent('keypress', 'TXL');
    })
    .then(function() {
        return page.evaluate(function() {
            return waitForDomElement(".suggestlist li[data-code=\"TXL\"]");
        });
    })
    .then(function() {
        return page.sendEvent('keypress', page.event.key.Tab);
    })
    .then(function() {
        return sendEvent('click','input[data-type="destination"]');
    })
    .then(function() {
        return page.sendEvent('keypress', 'LAX');
    })
    .then(function() {
        return page.evaluate(function() {
            return waitForDomElement(".suggestlist li[data-code=\"LAX\"]");
        });
    })
    .then(function() {
        return page.sendEvent('keypress', page.event.key.Tab);
    })
    .then(function() {
        return sendEvent('click', 'button.big');
    })
    .then(function() {
        return page.waitForLoaded();
    })
    .then(function() {
        return page.render('tui.png');
    })
    .then(function() {
        console.log("Success! Have a look at tui.pdf");
    }, function (error) {
        console.log("script failed :(" + (error.stack ? error.stack : error));
    })
    .then(phantom.exit);
