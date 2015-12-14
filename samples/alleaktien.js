"use strict";
var page = require('webpage').create();

//page.libraryPath += "/libs"
//page.viewportSize = { width: 1280, height: 2000 };
page.settings.userAgent = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/45.0.2454.101 Chrome/45.0.2454.101 Safari/537.36';


var sendEvent = function(type, selector) {
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

window.examineTree = function(jElm){
    console.log(jElm);
    if (jElm){
        jElm.children().each(function(){ 
            if ( $(this).is($('div.table_quotes'))){
                return;
            } else if ( $(this).find('div.table_quotes').length === 0  ) {
                console.log('--- '+$(this).prop("tagName"));
                $(this).remove();
            } else {
                console.log('+++ '+$(this).prop("tagName"));
                examineTree($(this));
            }
        })
    }
}
//console.log(examineTree.toString());
function doTheWork(i){
    return new Promise(function(resolve,reject){

        console.log('i:'+i);
        //page.open('http://www.finanzen.net/aktien/aktien_suche.asp?intpagenr='+i+'&inbranche=0&inland=1&inbillanz=0&inbillanzjahr=2001&inbillanzgrkl=2&inindex=0&infunndagrkl1=2&infunndagrkl2=2&infundamental1=0&infundamentaljahr1=2013&infundamental2=0&infundamentaljahr2=2013&insonstige=0')
        page.open('about:blank')
         .then(function(){
             return page.injectJs("jquery.min.js");
         })
         .then(function() { 
                 console.log("success!"); },
             function(error) { 
                 console.log("fail :(\n" + error); 
         })
         // .then(function(){
         //     return page.inject(examineTree);
         // })

         // .then(function() { 
         //         console.log("success!"); },
         //      function(error) { 
         //         console.log("fail :(\n" + error); 
         // })
        .then(function(){
            return page.evaluate(function(){
                return new Promise(function (accept) {
                    var examineTree = function(jElm){
                        jElm.children().each(function(){ 
                            if ( $(this).is($('div.table_quotes'))){
                                return;
                            } else if ( $(this).find('div.table_quotes').length === 0  ) {
                                console.log('--- '+$(this).prop("tagName"));
                                $(this).remove();
                            } else {
                                console.log('+++ '+$(this).prop("tagName"));
                                examineTree($(this));
                            }
                        })
                    }                    
                    var jElm = $('html');
                    examineTree(jElm);
                    console.log('ready DOM cleaning ....');
                    $('html').css('background-color','#69c773');
                    $('body').css('font-family','Helvetica, Arial');
                    $('a').css('text-decoration','none');
                    accept($('div.paging').offset().top);
                })
            })
        })
        .then(function(top) {
            console.log('top:'+top);
            page.viewportSize = { width: 1280, height: Math.ceil(top) };
            return phantom.wait(1000);
        })
        .then(function() {
            console.log('rendered');
            return page.render('alleaktien'+i+'.png');
        })
        .then(function(){
            console.log('I am ready');
            resolve();
        })

    })

}

var p = Promise.resolve();
for (let i=5; i<=7; i++) {
    p = p.then(() => doTheWork(i));
}
p.then(() => phantom.exit());

page.onPaint = function(dirtyRects, width, height, isPopup) {
  //console.log("Paint event of width = " + width + ", height = " + height + "! Was a popup? " + isPopup + ", Dirty rects = " + JSON.stringify(dirtyRects, 1));
};
