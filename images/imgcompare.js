var page = require('webpage').create();
page.settings.webSecurityEnabled = false;
page.viewportSize = { width: 1280, height: 2000 };
//page.clipRect = { top: 130, left: 200, width: 798, height:  553};
page.open("file://" + page.libraryPath + "/index.html")
//page.open('http://localhost:8082/tampere/awr?action=selMenu&menuId=519580750')
 	 .then(function() {
    	console.log("injecting!");
    	var ret = page.injectJs("jquery.min.js");
    	ret = page.injectJs("resemble.js");
    	return ret;
  	})
 	.then(function() { 
 	 		console.log("success!"); },
        function(error) { 
        	console.log("fail :(\n" + error); 
     })
    .then(function(){
        return page.evaluate(function(){
			return new Promise(function (accept) {
				resemble('tampere_test1.png').compareTo('tampere.png').onComplete(function(data){
					var diffImage = new Image();
					diffImage.src = data.getImageDataUrl();
					$('#image-diff').html(diffImage);
					//console.log($('html').html());
					accept(data.misMatchPercentage);
				});
			})
        })
    })
    .then(function(misMatchPercentage) {
        console.log('data.misMatchPercentage:'+misMatchPercentage)
    })
    .then(function() {
        return page.waitForSignal("onPaint");
    })
    .then(function() {
        return page.render('imgcompare.png');
    })
    .then(phantom.exit);

var print = function(o){
    var str='';

    for(var p in o){
        if(typeof o[p] == 'string'){
            str+= p + ': ' + o[p]+'; </br>';
        }else{
            str+= p + ': { </br>' + print(o[p]) + '}';
        }
    }

    return str;
}