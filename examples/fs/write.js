var fs = require('fs');
fs.write('/tmp/test.txt','hulu');
fs.write('test2.txt','blödsinn');
console.log('written');
fs.write('test2.txt','\n noch mehr blödsinn','a');
phantom.exit();
