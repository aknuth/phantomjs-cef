var page = require('webpage').create();
var system = require('system');

if (system.args.length < 3 || system.args.length > 5) {
    console.log('Usage: rasterize.js URL filename [paperwidth*paperheight|paperformat] [zoom]');
    console.log('  paper (pdf output) examples: "5in*7.5in", "10cm*20cm", "A4", "Letter"');
    console.log('  image (png/jpg output) examples: "1920px" entire page, window width 1920px');
    console.log('                                   "800px*600px" window, clipped to 800x600');
    // TODO
    phantom.exit(1);
}

var address = system.args[1];
var output = system.args[2];
var size;
page.viewportSize = { width: 600, height: 600 };
if (system.args.length > 3 && system.args[2].substr(-4) === ".pdf") {
    size = system.args[3].split('*');
    // TODO:
    page.paperSize = size.length === 2 ? { width: size[0], height: size[1], margin: '0px' }
                                        : { format: system.args[3], orientation: 'landscape', margin: '2cm' };
} else if (system.args.length > 3 && system.args[3].substr(-2) === "px") {
    size = system.args[3].split('*');
    if (size.length === 2) {
        var pageWidth = parseInt(size[0], 10);
        var pageHeight = parseInt(size[1], 10);
        page.viewportSize = { width: pageWidth, height: pageHeight };
        page.clipRect = { top: 0, left: 0, width: pageWidth, height: pageHeight };
    } else {
        console.log("size:" + system.args[3]);
        var pageWidth = parseInt(system.args[3], 10);
        var pageHeight = parseInt(pageWidth * 3/4, 10); // it's as good an assumption as any
        console.log ("pageHeight:" + pageHeight);
        page.viewportSize = { width: pageWidth, height: pageHeight };
    }
}

if (system.args.length > 4) {
    page.zoomFactor = parseInt(system.args[4]);
}

page.open(address)
    .then(function () {
        return page.render(output);
    })
    .catch(function(error) {
        console.log('Error: ' + error);
    })
    .then(phantom.exit);
