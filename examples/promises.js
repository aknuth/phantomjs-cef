var promise = new Promise(function(resolve, reject) {
  // do a thing, possibly async, then…

  var foo = 0;
  if (foo) {
    resolve("Stuff worked!");
  } else {
    reject(Error("It broke"));
  }
});

promise.then(
  function() { console.log("yes!"); },
  function() { console.log("meh!"); }
);
