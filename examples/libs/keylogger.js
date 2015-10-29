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
  console.log(JSON.stringify(obj, null, 2));
}

window.addEventListener('keydown', logKey, true, true);
window.addEventListener('keyup', logKey, true, true);
window.addEventListener('keypress', logKey, true, true);

console.log("key logger installed!");
