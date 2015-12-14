var page = require('webpage').create();

page.viewportSize = { width: 1280, height: 1024 };
page.settings.userAgent = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/45.0.2454.101 Chrome/45.0.2454.101 Safari/537.36';

function sendEvent(type, selector) {
    return page.evaluate(function(selector){
        var element = document.querySelector(selector);
        if (!element){
            throw Error("no element found with selector:" + selector);
        }
        return element.getBoundingClientRect();
    }, selector)
    .then(function(offset) {
        console.log(offset.left + 5);
        console.log(offset.top + 5);
        return page.sendEvent(type, offset.left + 5, offset.top + 5);
    })
}

page.onPaint = function(dirtyRects, width, height, isPopup) {
  //console.log("Paint event of width = " + width + ", height = " + height + "! Was a popup? " + isPopup + ", Dirty rects = " + JSON.stringify(dirtyRects, 1));
};

page.open('http://www.skyscanner.de/transport/fluge/duss/pmi/160421?adults=8')
//page.open('http://www.skyscanner.de/transport/fluge/pmi/duss/160426?adults=8')
    .then(function() {
        console.log("injecting!");
        return page.injectJs("../tampere/waitFor.js");
    })
    .then(function() { console.log("success!"); },
        function(error) { console.log("fail :(\n" + error); 
    })
    .then(function() {
        return page.waitForDomElement("span.wc-button-text");
    })
    .then(function() {
        return page.waitForSignal("onPaint");
    })
    .then(function() {
        sendEvent('click','a.acknowledge');
        sendEvent('click','div.close-button');
    })
    .then(function() {
        return page.evaluate(function(){
            return new Promise(function (accept) {
                var interval =  setInterval(function() {
                    var test = !$('div.day-progress-bar').is(':visible');
                    console.log(test);
                    console.log($('span.day-progress-providers').text());
                    //console.log($('section#day-section').html());
                    if (test){
                        clearInterval(interval);
                        accept();
                    }
                },500)
            });
        })
    })
    .then(function() {
        sendEvent('click','li[data-id=one_stop]');
        sendEvent('click','li[data-id=two_plus_stops]');
        //sendEvent('click','div.slider-handle-range');
        //page.sendEvent('click', 86, 538);
        //page.sendEvent('click', 106, 538);
        //page.sendEvent('click', 126, 538);
        page.sendEvent('click', 146, 538);
    })
    .then(function() {
        //return page.waitForSignal("onPaint");
        return phantom.wait(1000);
    })
    .then(function() {
        page.sendEvent('click', 166, 538);
    })
    .then(function() {
        //return page.waitForSignal("onPaint");
        return phantom.wait(1000);
    })
    .then(function() {
        console.log('rendered');
        return page.render('skyscanner.png');
    })
    .then(phantom.exit);
