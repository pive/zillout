var zlid3 = require('./build/Debug/zlid3');
var fs = require('fs');

var count = 0;
var lib   = new zlid3.Library('/Users/pveber/Music/Trap Them');
lib.loadImages(true);
lib.load(function(err, path, track, nfiles) {
	count++;
	if (err) {
		console.log("Error: " + err);	
	}
	else {
		if (typeof track.title === 'undefined') {
			console.log(path + " has no title.");
			console.log(track);
		}
		else {
			console.log("onLoad called (" + count + "/" + nfiles + "): " + track.title);
		}
	}
	if (count === nfiles) {
		var buf = lib.getAlbumImage(track.artist, track.album);
		fs.writeFileSync('test.png', buf.toString('binary'), 'binary');
	}
},
function(err, lib) {
	console.log("onComplete called.");
});
