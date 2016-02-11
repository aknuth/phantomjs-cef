"use strict";
let target = {a: "Hello, "};
let source = {b: "world!"};
// Objects are merged
Object.assign(target, source);
console.log(target.a + target.b);
phantom.exit();