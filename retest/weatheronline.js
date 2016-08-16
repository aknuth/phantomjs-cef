var page = require('webpage').create();
page.viewportSize = { width: 1920, height: 1200 };
page.clipRect = { top: 410, left: 450, width: 520, height: 545 };
page.settings.userAgent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.106 Safari/537.36';

page.open('http://www.wetteronline.de/regenradar/nordrhein-westfalen')
.then(()=>{
        return phantom.wait(2000);
})
.then(()=>{
        renderAndclickForward(1);
})

function renderAndclickForward(i){
        console.log(i);
        if (i===8){
                phantom.exit();
        } else {
                page.render('nrw'+i+'.png')
                .then(()=>{
                        return page.sendMouseEvent('click', 'div#nextButton');
                })
                .then(()=>{
                        return page.sendMouseEvent('click', 'html');
                })
                .then(()=>{
                        return phantom.wait(3000);
                })
                .then(()=>{
                        var j=i+1;
                        renderAndclickForward(j);
                })
        }
}