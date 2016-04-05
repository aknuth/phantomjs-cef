var page = require('webpage').create();
page.viewportSize = { width: 1920, height: 1200 };

var pagenr = 1;

page.onError = function(error) {
   //console.log(error);
};

page.open("http://confluence.arc42.org/display/templateEN/arc42+Template+%28English%29+-+Home")
    .then(function() {
        return page.sendMouseEvent('click', 'input[name=queryString][class*=medium-field]');
    })
    .then(function() {
        return page.sendEvent('keypress', 'Requirements');
    })	
    .then(function() {
        return page.sendMouseEvent('click', 'input[type=submit][class=aui-button]');
    })
    .then(function() {
        return page.waitForDomElement("a.search-result-link");
    })
	.then(function() {
        return printResults(page);
    })
    .then(function(){
        checkWeiterLink()
    })
    .catch(function(err) {
      console.log("FAIL!" + err);
      phantom.exit();
    })

function checkWeiterLink(){
    page.evaluate(()=>{
        let exist = (document.querySelectorAll('a.pagination-next').length===1);
        if (exist){
            console.log('Weiter Link exist');
            return true;
        } else {
            console.log('no "Weiter" Link exists');
            throw Error("no element inside page ...");
        }
    })
    .then(function(data) {
        iterateSearchResults();
    })
    .catch(function(err) {
      console.log("Finish");
      phantom.exit();
    })
}

function iterateSearchResults(){
    page.sendMouseEvent('click', 'a.pagination-next')
    .then(()=>{
        pagenr++;
        return page.waitForFunctionTrue(checkResultPage,pagenr);
    })
    .then(function() {
        return printResults(page);
    })
    .then(function(){
        checkWeiterLink()
    })
    .catch(function(err) {
      console.log(err);  
      phantom.exit();
    })
}

//This won't work - only named functions are allowed
//var fkt = function(nr){
function checkResultPage(nr){
    return document.querySelector('.pagination-curr').innerText==nr;
    //You can use that either because jQuery is loaded by the webpage
    //return $('.pagination-curr:contains('+nr+')').length===1;
}

function printResults(page){
    return page.evaluate(()=>{
        let nodes = document.querySelectorAll('a[class*=search-result-link]');
        [].forEach.call(nodes, (entry) => {
            console.log('----> '+entry.innerText);
        });
    })
}