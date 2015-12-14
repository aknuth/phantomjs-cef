"use strict";
class MyClass {
  constructor() { this.a = "Hello, "; }
  hello() { setInterval(() => console.log(this.a + "World!"), 1000); }
}
let myInstance = new MyClass();
myInstance.hello();
//phantom.exit();