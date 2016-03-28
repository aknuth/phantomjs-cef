"use strict";
var fs = require('fs');

function async(makeGenerator){
  return function (){
    var generator = makeGenerator.apply(this, arguments)
    
    function handle(result){ // { done: [Boolean], value: [Object] }
      if (result.done) return result.value
      
      return result.value.then(function (res){
        return handle(generator.next(res))
      }, function (err){
        return handle(generator.throw(err))
      })
    }
    
    return handle(generator.next())
  }
}

function doTheWork(i){
    return new Promise(function(resolve,reject){
    	var page = require('webpage').create();
    	page.settings.userAgent = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/45.0.2454.101 Chrome/45.0.2454.101 Safari/537.36';
        console.log('i:'+i);
        page.open('http://www.finanzen.ne'+(i===6?'':'t')+'/aktien/aktien_suche.asp?intpagenr='+i+'&inbranche=0&inland=1&inbillanz=0&inbillanzjahr=2001&inbillanzgrkl=2&inindex=0&infunndagrkl1=2&infunndagrkl2=2&infundamental1=0&infundamentaljahr1=2013&infundamental2=0&infundamentaljahr2=2013&insonstige=0')
	    .then(() => {return page.waitForDomElement("a.image_button_right");})
        .then(() => {
	    	console.log('injecting jQuery');
            return page.injectJs("jquery.min.js");
        })
	    .then(() => {
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
	    .then((top) => {
	        console.log('top:'+top);
	        page.viewportSize = { width: 1280, height: Math.ceil(top) };
	        return phantom.wait(1000);
	    })
	    .then(() => {
	        console.log('rendered');
	        return page.render('out/alleaktien'+i+'.png');
	    })
	    .then(() => {
	    	return page.evaluate(function(){
	    		return new Promise(function (accept) {
	    			accept( $('html')[0].outerHTML);
	    		})
	    	})
	    })
	    .then((html) => {
	    	fs.write('out/stocks'+i+'.txt',html);
	    })
	    .then(() => {
	    	console.log('I am ready');
	    	page.close();
	    	resolve();
	    })
	    .catch((reason) => {
	    	console.log('error - close page');
	    	page.close();
	    	reject();
	    });
	})
}

var p = Promise.resolve();
var iterable = [1,2,3,4,5, 6, 7];
var tries = 3;

whole(iterable,tries);

function whole(iterable, tries){
	console.log('tries:'+tries);
	if (tries===0){
		phantom.exit();
	}
	while (iterable.length>0){
		const m = iterable.pop();
	    p = p.then(() => doTheWork(m))
	    	.catch(() => {console.log('error ...');iterable.push(m);whole(iterable,tries-1)});
	}
}