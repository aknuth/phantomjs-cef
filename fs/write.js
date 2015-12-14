var fs = require('fs');
fs.write('test2.txt','blödsinn');
console.log('written');
fs.write('test2.txt','\n noch mehr blödsinn','a');
phantom.exit();