// List following and followers from several accounts

var users = ['PhantomJS',
        'ariyahidayat',
        'detronizator',
        'KDABQt',
        'lfranchi',
        'jonleighton',
        '_jamesmgreene',
        'Vitalliumm'];

var page = require('webpage').create();

page.onLoadFinished = function(status) {
  console.log("load finished: " + status);
};

page.onLoadStarted = function(url) {
  console.log("load started: " + url);
}

function follow(user, callback) {
    page.open('http://mobile.twitter.com/' + user, function (status) {
        if (status === 'fail') {
            console.log(user + ': ?');
        } else {
            page.evaluate(function () {
                return document.querySelector('.UserProfileHeader-statCount').innerText;
            }).then(function(data) {
              console.log(user + ': ' + data);
              callback.apply();
            });
        }
    });
}

function process() {
    if (users.length > 0) {
        var user = users[0];
        users.splice(0, 1);
        follow(user, process);
    } else {
        phantom.exit();
    }
}

process();

