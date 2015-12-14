var fs = require('fs');
var e = fs.exists('/home/aknuth/sl1.js');
console.log(e);
var t = fs.copy('/home/aknuth/sl1.js','/home/aknuth/repo');
console.log(t);
var t = fs.copy('/home/aknuth/Dokumente','/home/aknuth/repo/docs');
console.log(t);
phantom.exit();