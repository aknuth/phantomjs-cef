"use strict";
// Code with spread operator
function myFunction (a,b,c) {
   console.log(a);
   console.log(b);
   console.log(c);
}

let argsInArray = ["Hi ", "Spread ", "operator!"];
myFunction(...argsInArray);
phantom.exit();