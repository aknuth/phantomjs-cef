"use strict";

var system = require('system'),
	host = 'localhost';

if (system.args[1]){
	host = system.args[1];
}

var wsUrl = 'ws://'+host+':8887',
	ws = new WebSocket('ws://'+host+':8887'),
    pageMap = new Map();

var setToValue = function (obj, value, path) {
    var path = path.split('.'),
        parent = obj,
        selector;

    for (var i = 0; i < path.length -1; i += 1) {
        console.log(parent);
        parent = parent[path[i]];
    }

    parent[path[path.length-1]] = value;
}

ws.onerror = function (error) {
  console.log('WebSocket Error ' + error);
};

ws.onopen = function() {
  console.log('open');
	  var result = {};
	  result.event='ACK';
	  ws.send(JSON.stringify(result));// Sends a message.
}

ws.onmessage = function(e) {
	try {
	  var message = e.data;
	  console.log(message);
	  var json = JSON.parse(message);
	  callFunction(json);
	} catch (e) {
	  console.log(e);
	  return;
	}
}

ws.onclose = function() {
  console.log('close')
  phantom.exit();
}

var callFunction = function(json) {
	var command = json['command'];
	var args = json['args'];
	var prefix = '_instance.';
	if (!json.page){
	  prefix = '';
	} else if (!json.page.id){
	  var _instance = new Page(json.page.uid);
	  pageMap.set(json.page.uid, _instance);
	} else {
	  var _instance = pageMap.get(json.page.uid);  
	  console.log('_instance:'+_instance);
	}
	for (var t in json.page){
	  if (json.page.hasOwnProperty(t)) {
		setToValue(_instance.page,json.page[t],t);
	  }
    }
	var tmp = prefix + command + '(';
	for (var i = 0; i < args.length; i++) {
		tmp = tmp + '"' + args[i] + '",';
	}
	if (args.length===0){
		tmp = tmp + ');'
	} else {
		tmp = tmp.substring(0, tmp.length-1) + ');'
	}
	console.log('--> '+tmp);
	eval(tmp);
}

var exit = function(){
	console.log('exit phantomjs ...');
	phantom.exit();
}

class Page {
	
	constructor(uid) { 
	    this.page=require('webpage').create();
		this.uid=uid;
		this.paint = [0];
		
		this.page.onLoadFinished = (status,url) => {
			console.log('####onLoadFinished | page.id='+this.page.getId());
			var result = this.getResult('onLoadFinished');
		    ws.send(JSON.stringify(result));
		}

		this.page.onPaint = (dirtyRects, width, height, isPopup) => {
			//console.log("Paint event | page.id="+this.page.getId());
			var number = this.paint[0];
			this.paint[0]=number+1;
			var result = this.getResult('onPaint');
			ws.send(JSON.stringify(result))
		};
		this.page.onResourceReceived = (response) => {
			console.log("####onResourceReceived | page.id="+this.page.getId());
			var result = this.getResult('onResourceReceived');
			ws.send(JSON.stringify(result))
		}
		
		this.page.onLoadStarted = (url) => {
			console.log('####onLoadStarted | '+url+' | page.id='+this.page.getId());
			var result = this.getResult('onLoadStarted');
		    ws.send(JSON.stringify(result));
		}
		
	}
	
	pollPaint(duration, retries){
		var d = parseInt(duration);
		var r = parseInt(retries);
		this.paint[0]=0;
		
		Array.observe(this.paint, (changes) => {
		  console.log("this.paint[0]:"+this.paint[0]);
		  if (this.paint[0]===r){
			console.log("pollpaint -> number changes");
			var result = this.getResult('pollPaint');
			ws.send(JSON.stringify(result))
			Array.unobserve(this.paint,()=>{});
		  }
		});
		setTimeout(()=>{
			console.log("pollpaint -> timeout");
			var result = this.getResult('pollPaint');
			ws.send(JSON.stringify(result))
			Array.unobserve(this.paint,()=>{});
		},d)
	}
	
	start(url) {
		console.log("start: " + url);
		this.page.open(url);
	}
	
	capture() {
		console.log("capture:");
		this.page.renderBase64("PNG")
		.then((data)=>{
			var result = this.getResult('Image');
			result.pic = data;
			ws.send(JSON.stringify(result));
		})
		.catch(function(error) {
	        console.log('Error: ' + error);
	    })
	}
	
	execute(executable) {
		var exec = atob(executable);
		console.log('exec:'+exec);
		this.page.evaluate(function(exec){
			return Function(exec)();  
		},exec)
		.then((data)=>{
			var result = this.getResult('execute');
			result.text = JSON.stringify(data);
		    ws.send(JSON.stringify(result));
		})
	}
	
	text(selector) {
		var s = atob(selector);
		console.log('selector:'+s);
		this.page.evaluate(function(s){
			var elm = document.querySelector(s);
			if (elm.nodeName === 'INPUT'){
				return elm.value;
			} else {
				return elm.textContent;
			}
		},s)
		.then((data)=>{
			var result = this.getResult('text');
			result.text = data;
		    ws.send(JSON.stringify(result));
		})
		.catch((error) => {
			var result = this.getResult('text');
			result.error = true;
			if (typeof error === 'string'){
				result.errormsg = error;
			} else {
				result.errormsg = error.name + ': ' + error.message;
			}
			ws.send(JSON.stringify(result));
	    })
	}

	list(selector) {
		var s = atob(selector);
		console.log('selector:'+s);
		this.page.evaluate(function(s){
			  "use strict";
			  let arr=[];
			  let list = document.querySelectorAll(s);
			  for (let item of Array.from(list)) {
				if (item.nodeName === 'INPUT'){
				    arr.push(item.value);
				} else {
					arr.push(item.textContent);
				}
				  
			  };
			  return arr;			
		},s)
		.then((data)=>{
			var result = this.getResult('list');
			result.array = data;
		    ws.send(JSON.stringify(result));
		})
		.catch((error) => {
			var result = this.getResult('list');
			result.error = true;
			result.errormsg = error.name + ': ' + error.message;
			ws.send(JSON.stringify(result));
	    })
	}
	
	test(testFx) {
		var testFunction = atob(testFx);
		console.log('testFunction:'+testFunction);
		this.page.evaluate(function(testFunction){
		    return Function(testFunction)();
		},testFunction)
		.then((data)=>{
			var result = this.getResult('test');
			if (typeof data === 'boolean'){
				result.test = data;
			} else {
				throw Error('function does not return a boolean - instead:'+(typeof data));
			}
		    ws.send(JSON.stringify(result));
		})
		.catch((error) => {
			var result = this.getResult('test');
			result.error = true;
			result.errormsg = error.name + ': ' + error.message;
			ws.send(JSON.stringify(result));
	    })
	}
	
	testScript(script) {
		console.log('script:'+script);
		this.page.injectJs(script)
		.then((data) => { 
				var result = this.getResult('testScript');
				if (typeof data === 'boolean'){
					result.test = data;
				} else {
					throw Error('function does not return a boolean - instead:'+(typeof data));
				}
			    ws.send(JSON.stringify(result));
		    },
             (error) => { 
                 console.log("fail :(\n" + error);
     			 var result = this.getResult('testScript');
    			 result.error = true;
    			 result.errormsg = error.name + ': ' + error.message;
    			 ws.send(JSON.stringify(result));
         })
         .catch((error) => {
			var result = this.getResult('testScript');
			result.error = true;
			result.errormsg = error.name + ': ' + error.message;
			ws.send(JSON.stringify(result));
	    })
		
	}
	
	sendKeyEvent(type, value) {
		if (this.page.event.key[value]){
	   		var v = this.page.event.key[value];
	   	} else {
	   		v = value;
	   	}
		this.page.sendEvent(type, v)
	    .then(() => {
			var result = this.getResult('KeyEventFinished');
		    ws.send(JSON.stringify(result));
	    })
	}
	
	sendMouseEvent(type, selector) {
	    this.page.evaluate(function(selector){
	        var sel = atob(selector);
	        console.log('Selector: '+sel);
	    	var element = document.querySelector(sel);
	        if (!element){
	            throw Error("no element found with selector:" + sel);
	        }
	        return element.getBoundingClientRect();
	    }, selector)
	    .then((offset) => {
	        console.log('sendEvent:'+offset.left+":"+offset.top)
	    	return this.page.sendEvent(type, offset.left + 5, offset.top + 5);
	    })
	    .then(() => {
			var result = this.getResult('MouseEventFinished');
		    ws.send(JSON.stringify(result));
	    })
	    //.catch(errorResult);
	}
	
	waitForDomElement(selector){
		var s = atob(selector);
		this.page.waitForDomElement(s)
		.then(()=>{
			var result = this.getResult('waitForDomElement');
		    ws.send(JSON.stringify(result));
		})
	}
	
	getResult(ev){
		var result = {};
		result.event=ev;
		var tmpPage = JSON.stringify(this.page);
		var p = JSON.parse(tmpPage);
		p.event=undefined;
		p.uid=this.uid;
		p.id=this.page.getId();
		var viewportSize = {};
		viewportSize.width=this.page.viewportSize.width;
		viewportSize.height=this.page.viewportSize.height;
		p.viewportSize=viewportSize;
		result.page=p;
		return result;
	}
	
}