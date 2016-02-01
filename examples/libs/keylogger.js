function logKey(event) {
  var obj = {};
  for (var k in event) {
    if (k.toUpperCase() === k || k === "timeStamp") {
      continue;
    }
    try {
      var v = JSON.stringify(event[k]);
    } catch(err) { continue; }
    obj[String(k)] = v;
  }
  console.log(event.type + ": " + JSON.stringify(obj, null, 2));
}

window.addEventListener('keydown', logKey, true, true);
window.addEventListener('keyup', logKey, true, true);
window.addEventListener('keypress', logKey, true, true);
window.addEventListener('mousedown', logKey, true, true);
window.addEventListener('mouseup', logKey, true, true);
window.addEventListener('mousemove', logKey, true, true);
window.addEventListener('click', logKey, true, true);
window.addEventListener('dblclick', logKey, true, true);

console.log("key logger installed!");
