'use strict';

function* generator() {
  console.log(0);

  yield 1;
  yield 2;
  yield 3;
}

let iterator = generator();
let current;

do {
  current = iterator.next();
  console.log(JSON.stringify(current));
} while (!current.done);

phantom.exit();