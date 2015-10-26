// List following and followers from several accounts

var users = ['PhantomJS',
        'ariyahidayat',
        'detronizator',
        'KDABQt',
        'lfranchi',
        'jonleighton',
        '_jamesmgreene',
        'Vitalliumm'];

function follow(user, callback) {
    var page = require('webpage').create();
    page.open('http://mobile.twitter.com/' + user, function (status) {
        if (status === 'fail') {
            console.log(user + ': ?');
        } else {
            page.evaluate(function () {
                return document.querySelector('.UserProfileHeader-statCount').innerText;
            }, function(data) {
              console.log(user + ': ' + data);
              page.close();
              callback(user);
            });
        }
    });
}

function process(user) {
    users.splice(users.indexOf(user), 1);
    if (!users.length) {
        phantom.exit();
    }
}

for (var idx in users) {
  follow(users[idx], process);
}

