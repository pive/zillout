var zlid3   = require('../zlid3/build/Debug/zlid3');
var express = require('express');
var io      = require('socket.io');
var http    = require('http');
var fs      = require('fs');
var extend  = require('extend');

var app    = express();
var server = http.createServer(app);

var zillout = {
	path:     __dirname + '/private/zillout.json',
	root:     '/Users/pveber/Music',
	port:     9090,
	loaded:   false,
	loading:  false,
	version:  3,
	library:  null,
	iosocket: null,
	error:    false
};

var saveConfig = function(aftercb) {
	var config = {
		root: zillout.root,
		port: zillout.port,
		version: zillout.version
	}
	console.log('saveConfig - config: ' + JSON.stringify(config));
	fs.writeFile(zillout.path, JSON.stringify(config), function(err) {
		if (err) {
			return console.log("Error writing: " + err);
		}
		if (typeof aftercb === 'function') {
			aftercb();
		}
	});
}

var bootConfig = function() {
	if (fs.existsSync(zillout.path)) {
		fs.readFile(zillout.path, function(err, data) {
			if (err) {
				return console.log("Error reading: " + err);
			}
			var json = JSON.parse(data);
			if (!json) {
				return console.log("Error: could not parse " + zillout.path);
			}
			zillout = extend(true, zillout, json);
			console.log('bootConfig - zillout: ' + JSON.stringify(zillout));
			bootServer();
		});
	}
	else {
		savePrivate(bootServer);
	}
};

var bootServer = function() {

	loadLibrary(zillout.root);

	app.use(express.static(__dirname + '/public'));

	var ioserv = io.listen(server);
	ioserv.on('connection', function(socket) {
		console.log('IO - user connected');
		zillout.iosocket = socket;
		var status = {
			loaded:  zillout.loaded,
			loading: zillout.loading,
			path:    zillout.root
		};
		console.log('IO - status: ' + JSON.stringify(status));
		zillout.iosocket.emit('status', status);
		zillout.iosocket.on('command', function(msg) {
			console.log('IO - command received: ' + JSON.stringify(msg));
			if (msg.cmd && msg.cmd === 'load' && !zillout.loading) {
				loadLibrary(msg.path, socket);
			}
			else if (msg.cmd && msg.cmd === 'disconnect') {
				zillout.iosocket.disconnect();
			}
		});
	});
	server.listen(zillout.port);

	app.param('trackid', function (req, res, next, tid) {
		if (/^\d+$/.exec(String(tid))) {
			next();
		}
	});
	app.get('/tracks/:trackid/image', function(req, res) {
		image = zillout.library.getTrackImage(parseInt(req.params.trackid));
		if (image) {
			res.type(image.type);
			return res.send(image.buffer);
		}
		res.status(204).send();
	});
	app.get('/tracks/:trackid/audio', function(req, res) {
		path = zillout.library.getTrackFile(parseInt(req.params.trackid));
		if (path) {
			return res.sendFile(path);
		}
		res.status(204).send();
	});
	app.get('/albums/:artist/:title/image', function(req, res) {
		image = zillout.library.getAlbumImage(req.params.artist, req.params.title);
		if (image) {
			res.type(image.type);
			return res.send(image.buffer);
		}
		res.status(204).send();
	});
	app.get('/tracks', function(req, res) {
		tracks = zillout.library.getTracks();
		res.send(tracks);
	});
	app.get('/config', function(req, res) {
		res.send(JSON.stringify( { path: zillout.root } ));
	});
};

var loadLibrary = function(path) {

	console.log('loadLibrary - will load: ' + path);

	zillout.loading = true;
	zillout.loaded  = false;
	zillout.error   = false;
	zillout.library = new zlid3.Library(path);
	zillout.library.load(
		/* on load file */
		function(trackid, nprocessed, nfiles) {
			if (zillout.iosocket !== null) {
				zillout.iosocket.emit('loadtrack', { count: nprocessed, total: nfiles });
			}
		},
		/* on load complete */
		function() {
			console.log('loadLibrary - complete.');
			zillout.root    = path;
			zillout.loaded  = true;
			zillout.loading = false;
			zillout.error   = false;
			saveConfig(function() {
				if (zillout.iosocket !== null) {
					zillout.iosocket.emit('loaded', {});
				}
			});
		},
		function(err) {
			console.log('loadLibrary - error: '+ err);
			zillout.error   = err;
			zillout.loading = false;
			zillout.loaded  = false;
			if (zillout.iosocket !== null) {
				zillout.iosocket.emit('loaderr', { error: err });
			}
		}
	);
};

(function() {
	bootConfig();
})();

