var fs = require('fs');
fs.makeDirectory('/home/aknuth/tmp');
fs.touch('/home/aknuth/tmp/gg');
console.log('size of gg:'+fs.size('/home/aknuth/tmp'));
console.log(fs.tempPath());
phantom.exit();