// List following and followers from several accounts

var page = require('webpage').create();
page.open('http://ariya.ofilabs.com/2016/02/easy-docker-on-os-x.html')
.then(function(){
    return page.evaluate(function () {
        document.querySelector('div.related-posts').remove();
        document.querySelector('header').remove();
        document.querySelector('#sidebar1').remove();
        document.querySelector('#disqus_thread').remove();
        document.querySelector('footer').remove();
        document.querySelector('#wpstats').remove();
    })
})
.then(function(data){
    console.log('render');
    return page.render('blog.pdf');
})
.catch(function(error) {
        console.log('Error: ' + error);
    })
.then(phantom.exit);

