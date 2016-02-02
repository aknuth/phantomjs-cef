var fs = require('fs');

var path = fs.tempPath() + "/phantomjs";
var subPath = path + "/1/2/3";
fs.makeTree(subPath);
fs.touch(subPath + '/1');
fs.touch(subPath + '/2');
fs.touch(subPath + '/3');

var path2 = fs.tempPath() + "/phantomjs2";
fs.copy(path, path2);

console.log(fs.list(path2 + "/1/2/3"));

fs.remove(path);
fs.remove(path2);
