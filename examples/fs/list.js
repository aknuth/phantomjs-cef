var fs = require('fs');
var list = fs.list('.');
for(var x = 0; x < list.length; x++){
	console.log("Content: "+list[x]);
}
phantom.exit();