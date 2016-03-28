var page = require('webpage').create();
page.viewportSize = {width: 1920, height: 1200};
page.open("chrome://version")
    .then(() => {
        return phantom.wait(1000);      
    })
    .then(() => {
    	return page.evaluate(()=>{
    		let data = [];
    		let tr = document.querySelectorAll('tr');
    		//tr isn't an array - it's a NodeList -> so we try this hack (https://css-tricks.com/snippets/javascript/loop-queryselectorall-matches)
    		[].forEach.call(tr, (entry) => {
    			data.push(entry.innerText);
    		});
    		return data;
        })
    })
    .then((data) => {
        data.forEach((entry)=>{console.log(entry.replace('\t',':'))});
    	      
    })
    .catch((error) => {
        console.log('Error: ' + error);
    })
    .then(phantom.exit);