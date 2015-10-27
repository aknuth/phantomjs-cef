phantom.onError = function(msg, stack, url, line, column, error) {
  console.log(msg);
  console.log(stack);
  console.log(url);
  console.log(line);
  console.log(column);
  console.log(error);
};

function foo() {
  asdfasdf();
}

function bar() { foo(); }

bar();
