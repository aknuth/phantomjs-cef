"use strict";
var page = require('webpage').create();
page.open("file://" + page.libraryPath + "/index.html")
.then(function() {
	page.evaluate(function(){
		"use strict";
		class MyClass {
  			constructor() { this.a = "Hello, "; }
  			hello() { setInterval(() => console.log(this.a + "World!"), 1000); }
		}
		let myInstance = new MyClass();
		myInstance.hello();
	})
})