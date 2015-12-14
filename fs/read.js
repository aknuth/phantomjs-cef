var fs = require('fs');
var s = fs.read('test2.txt');
console.log("read: "+s);
phantom.exit();