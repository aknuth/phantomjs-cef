var fs = require('fs');
var s = fs.lastModified('test2.txt');
console.log("lastModified: "+s);
phantom.exit();