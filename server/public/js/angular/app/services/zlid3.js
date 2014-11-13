function zlId3($rootScope, $http) {

	var zlId3 = {};

	console.log('zlId3 - booting.');

	zlId3.client = null;
	zlId3.tracks = [];
	zlId3.status = {
		loaded:  false,
		loading: false,
		path:    null
	};

	zlId3.listeners = {
		load:      [],
		loading:   [],
		loaded:    [],
		loaderr:   [],
		tracklist: []
	};

	zlId3.emit = function() {
		var ev   = arguments[0];
		if (ev in zlId3.listeners) {
			var args = [];
			for (var i = 1; i < arguments.length; i++) {
				args.push(arguments[i]);
			}
			zlId3.listeners[ev].forEach(function(cb) {
				setTimeout(function() { $rootScope.$apply(cb.apply(cb, args)); }, 0);
			});
		}
	}

	zlId3.on = function(ev, cb) {
		if (ev in zlId3.listeners) {
			zlId3.listeners[ev].push(cb);
		}
	}

	zlId3.load = function(path) {

		if (zlId3.client !== null) {
			if (zlId3.status.loaded && zlId3.status.path === path) {
				console.log("Won't reload library, path is not changed: " + path);
				return zlId3.emit('tracklist', path, zlId3.tracks);
			}

			console.log("Will reload library, path changed: " + path);
			zlId3.client.emit('command', { cmd: "load", path: path });
			zlId3.emit('load', path);

			zlId3.status.loaded  = false;
			zlId3.status.path    = path;
		}
		else {
			zlId3.connect();
		}
	}

	zlId3.tracklist = function(path) {
		var req = $http.get('/tracks');
		req.success(function(data) {
			zlId3.tracks = data;
			zlId3.emit('tracklist', path, zlId3.tracks);
		});
		req.error(function(data, status) {
			zlId3.tracks = [];
			zlId3.emit('loaderr', 'Could not get track list [HTTP ' + status + ', ' + data + ']');
		});
	}

	zlId3.connect = function() {

		zlId3.client = io.connect();
		zlId3.client.on('status', function(status) {
			zlId3.status = status;
			zlId3.emit('load', status.path);
			if (status.loaded) {
				zlId3.emit('loaded', status.path);
				zlId3.tracklist(status.path);
			}
			zlId3.client.on('loadtrack', function(progress) {
				zlId3.emit('loading', progress);
				zlId3.status.loading = true;
			});
			zlId3.client.on('loaded', function() {
				zlId3.emit('loaded', status.path);
				zlId3.status.loading = false;
				zlId3.status.loaded  = true;
				zlId3.tracklist(status.path);
			});
			zlId3.client.on('loaderr', function(err) {
				zlId3.emit('loaderr', err);
				zlId3.status.loading = false;
				zlId3.status.loaded  = false;
			});
		});
	}

	return zlId3;
};
angular.module('zillout').factory('zlId3', zlId3);
