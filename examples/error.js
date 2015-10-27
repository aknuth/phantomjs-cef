phantom.onError2 = function(msg, stack, url, line, column, error) {
  console.log(msg);
  console.log(stack);
  console.log(url);
  console.log(line);
};

function foo() {
  asdfasdf();
}

function bar() { foo(); }

bar();
