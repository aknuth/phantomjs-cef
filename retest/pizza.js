// Find pizza in Mountain View using Yelp

var page = require('webpage').create(),
    url = 'https://www.yelp.com/search?find_desc=pizza&find_loc=Amsterdam%2C+Noord-Holland%2C+Netherlands';

page.open(url)
    .then(function () {
        return page.evaluate(function() {
            var list = document.querySelectorAll('address'), pizza = [], i;
            for (i = 0; i < list.length; i++) {
                pizza.push(list[i].innerText);
            }
            return pizza;
        });
    })
    .then(function (results) {
        console.log(results.join('\n'));
    })
    .then(phantom.exit)
