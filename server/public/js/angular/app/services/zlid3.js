function zlId3($rootScope, $http, zlEmitter) {

	var zlId3 = zlEmitter.new([
		'ready',
		'loading',
		'loaded',
		'loaderr',
		'tracklist'
		]);

	zlId3.client = null;
	zlId3.tracks = [];
	zlId3.status = {
		boot:    true,
		ready:   false,
		loaded:  false,
		loading: false,
		path:    null
	};

	console.log('zlId3 - booting service - status: ' + JSON.stringify(zlId3.status));

	zlId3.load = function(path) {

		/* check that service is ready */
		if (!zlId3.status.ready && !zlId3.status.boot) {
			console.log("zlId3.load - Won't reload library, service is not ready.");
			console.log('zlId3 emit loaderr event.');
			zlId3.emit('loaderr', 'Service is not ready.');
			return false;
		}

		/* if path is defined, check that server is able to receive the command and path changed */
		if (path) {
			if (!zlId3.status.loaded) {
				console.log("zlId3.load - Won't reload library, service is still loading: " + zlId3.status.path);
				console.log('zlId3 emit loaderr event.');
				zlId3.emit('loaderr', 'Service is still loading.');
				return false;
			}
			else if (zlId3.status.path === path) {
				console.log("zlId3.load - Won't reload library, path is not changed: " + path);
				return false;
			}
		}

		/* reset service status */
		zlId3.status = {
			ready:   false,
			loaded:  false,
			loading: false,
			path:    null
		};

		/* connect to IO server */
		console.log('zlId3.load - IO connect...');
		zlId3.client = io.connect();

		/* IO connect status handler */
		zlId3.client.on('status', function(status) {

			console.log('zlId3 io status handler - connected, received status: ' + JSON.stringify(status) + ', emit ready event.');
			zlId3.status = status;
			zlId3.status.ready = true;
			zlId3.emit('ready');

			if (status.loaded) {
				console.log('zlId3 io status handler - received status: loaded.');
				if (path) {
					console.log('zlId3 io status handler - should load new path, status loaded, proceed with load command.');
					console.log("zlId3 io status handler - Will reload library, path changed: " + path);
					zlId3.client.emit('command', { cmd: "load", path: path });
					zlId3.status.loaded  = false;
					zlId3.status.path    = path;
					zlId3.emit('load', path);
				}
				else {
					console.log('zlId3 io status handler - status loaded, emit loaded event and get track list, then disconnect.');
					zlId3.emit('loaded', status.path);
					zlId3.tracklist(status.path);
					zlId3.client.emit('disconnect');
				}
			}
			else {
				zlId3.client.on('loadtrack', function(progress) {
					zlId3.emit('loading', progress);
					zlId3.status.loading = true;
				});

				zlId3.client.on('loaded', function() {
					console.log('zlId3 io loaded handler - emit loaded event and get track list, then disconnect.');
					zlId3.emit('loaded', status.path);
					zlId3.status.loading = false;
					zlId3.status.loaded  = true;
					zlId3.tracklist(status.path);
					zlId3.client.emit('disconnect');
				});

				zlId3.client.on('loaderr', function(err) {
					console.log('zlId3 io loaderr handler - emit loaderr event, then disconnect.');
					zlId3.emit('loaderr', err);
					zlId3.status.loading = false;
					zlId3.status.loaded  = false;
					zlId3.client.emit('disconnect');
				});
			}
		});

		/* IO connect disconnect handler */
		zlId3.client.on('disconnect', function() {

			zlId3.client.removeAllListeners();
			if (!zlId3.status.loaded) {
				console.log('zlId3 io disconnect handler - not loaded, trigger error');
				zlId3.emit('loaderr', 'Inexpectedly disconnected from server.');
			}
			else {
				console.log('zlId3 io disconnect handler - expected closed connection.');
				zlId3.client = null;
			}
		
		});

		zlId3.client.on('connect_error', function() {
			console.log('zlId3 io connect_error handler - trigger error');
			zlId3.emit('loaderr', 'Could not connect to server.');
		});

		zlId3.client.on('connect_timeout', function() {
			console.log('zlId3 io connect_timeout handler - trigger error');
			zlId3.emit('loaderr', 'Server connection timeout.');
		});

		return true;
	}

	zlId3.tracklist = function(path) {

		console.log('zlId3.tracklist - get /tracks...');
		var req = $http.get('/tracks');
		req.success(function(data) {
			console.log('zlId3.tracklist - success, emit tracklist event.');
			zlId3.tracks = data;
			zlId3.emit('tracklist', path, zlId3.tracks);
		});
		req.error(function(data, status) {
			console.log('zlId3.tracklist - failed [HTTP ' + status + ', ' + data + ']');
			zlId3.tracks = [];
			zlId3.emit('loaderr', 'Could not get track list [HTTP ' + status + ', ' + data + ']');
		});
	}

	console.log('zlId3 boot calling load...');
	zlId3.load();

	return zlId3;
};
angular.module('zillout').factory('zlId3', zlId3);
