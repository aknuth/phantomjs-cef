var promise = new Promise(function(resolve, reject) {
  // do a thing, possibly async, then…

  setTimeout(function() {
    var foo = 0;
    if (foo) {
      resolve("Stuff worked!");
    } else {
      reject(Error("It broke"));
    }
  }, 500);
});

promise.then(
  function() { console.log("yes!"); },
  function() { console.log("meh!"); }
);

promise.then(
  function() { console.log("ye2s!"); },
  function() { console.log("me2h!"); }
).then(
  function() { console.log("ye3s!"); },
  function() { console.log("me3h!"); }
);

promise.then( phantom.exit, phantom.exit );
