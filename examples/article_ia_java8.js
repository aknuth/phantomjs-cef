// List following and followers from several accounts

var page = require('webpage').create();

page.viewportSize = { width: 1920, height: 1200 };
page.clipRect = { top: 0, left: 0, width: 1920, height: 1200 };
page.paperSize = {
  format: 'A4',
  //orientation: 'landscape',
  margin: '3cm'
};

page.open('http://www.informatik-aktuell.de/entwicklung/programmiersprachen/java-8-im-praxiseinsatz.html')
.then(function(){
    return page.evaluate(function () {
        $('#sidebar').remove();$('#header').remove();$('#footer').remove();$('#copyright').remove();
        $('#breadcrumb').remove();$('#topnavi').remove();
        $('div[style=clear\\:left]').nextAll().remove();
        $('div[style=clear\\:left]').remove();
        $('#content-wrap').unwrap();
        $('#content').unwrap();
        $('#content').css('margin','10px');
        //$('pre').css('page-break-inside', 'avoid');
        $('pre').css('overflow','visible');
    })
})
.then(function(data){
    return page.render('java8.pdf');
})
.catch(function(error) {
        console.log('Error: ' + error);
    })
.then(phantom.exit);

