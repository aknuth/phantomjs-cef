"use strict";
var page = require('webpage').create();

function examineTree(jElm){
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

var test = function test(a){
	console.log(a);
}

console.log(test.toString());
console.log(test.name);

page.open('about:blank')
 .then(function(){
     return page.injectJs("jquery.min.js");
 })
 .then(function() { 
         console.log("success!"); },
     function(error) { 
         console.log("fail :(\n" + error); 
 })
 .then(function(){
      return page.inject(examineTree);
  })

 .then(function() { 
          console.log("success!"); },
       function(error) { 
          console.log("fail :(\n" + error); 
  })
/*  .then(function(){
      return page.injectJs("examineTree.js");
  })
*/ .then(function(){
    return page.evaluate(function(){
            //test('blavl');
            var jElm = $('html');
            examineTree(jElm);
            console.log('ready DOM cleaning ....');
    })
 })

