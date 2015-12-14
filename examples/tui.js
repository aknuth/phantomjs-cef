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

page.onPaint = function(dirtyRects, width, height, isPopup) {
  //console.log("Paint event of width = " + width + ", height = " + height + "! Was a popup? " + isPopup + ", Dirty rects = " + JSON.stringify(dirtyRects, 1));
};

page.open('https://www.tuifly.com/Select.aspx')
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
        return page.waitForDomElement(".suggestlist li[data-code=\"TXL\"]");
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
        return page.waitForDomElement(".suggestlist li[data-code=\"LAX\"]");
    })
    .then(function() {
        return page.sendEvent('keypress', page.event.key.Tab);
    })
    //.then(function() {
    //    return page.sendEvent('keypress', page.event.key.space);
    //})
    .then(function() {
        return sendEvent('click', 'label.js-tfl-chooser');
    })
    .then(function(){
        phantom.wait(2000);
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
        console.log("Success! Have a look at tui.png");
    }, function (error) {
        console.log("script failed :(" + (error.stack ? error.stack : error));
    })
    .then(phantom.exit);
