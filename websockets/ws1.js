var page = require('webpage').create();
var ws = new WebSocket('ws://localhost:8887')
ws.onopen = function() {
  console.log('open')
  var result = {};
  result.returnValue=2;
  result.content='ACK';
  ws.send(JSON.stringify(result));// Sends a message.
}
ws.onmessage = function(e) {
	try {
	  var message = e.data;
	  console.log(message);
	  var json = JSON.parse(message);
	  console.log(json.command);
	  execute(json);
	} catch (e) {
	  console.log('This doesn\'t look like a valid JSON: ', e);
	  return;
	}
}
ws.onclose = function() {
  console.log('close')
  phantom.exit();
}
