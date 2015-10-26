function createPromise(pass)
{
  return new Promise(function(resolve, reject) {
  // do a thing, possibly async, then…
  setTimeout(function() {
    if (pass) {
      resolve("Stuff worked!");
    } else {
      reject(Error("It broke"));
    }
  }, 500);
});
}

var promise = createPromise(true);

promise.then(
  function() { console.log("yes!" + JSON.stringify(arguments)); },
  function() { console.log("meh!" + JSON.stringify(arguments)); }
);

promise.then(
  function() { console.log("ye2s!" + JSON.stringify(arguments)); return createPromise(false); },
  function() { console.log("me2h!" + JSON.stringify(arguments)); }
).then(
  function() { console.log("ye3s!" + JSON.stringify(arguments));},
  function() { console.log("me3h!" + JSON.stringify(arguments)); }
).then( phantom.exit, phantom.exit );
