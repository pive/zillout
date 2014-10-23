var zlid3 = require('./build/Debug/zlid3');

var count = 0;
var lib = new zlid3.Library('/Users/pveber/Music');
lib.load(function(err, path, track, nfiles) {
	count++;
	console.log("onLoad called (" + count + "/" + nfiles + "): " + path);
},
function(err, lib) {
	console.log("onComplete called.");
});
