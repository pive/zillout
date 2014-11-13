function zlPlayer($rootScope, $http) {

	var zlPlayer = {};

	zlPlayer.ready = false;
	zlPlayer.sm2   = soundManager.setup({
		url: '/media/soundmanager/swf/',
  		flashVersion: 9,
		preferFlash: false,
		onready: function() {
			zlPlayer.ready = true;
			console.log('zlPlayer.sm2 ready.');
		}
	});
	zlPlayer.sound     = null;
	zlPlayer.listeners = {
		library:  [],
		load:     [],
		play:     [],
		pause:    [],
		stop:     [],
		end:      [],
		progress: []
	};

	zlPlayer.library = {
		tracks:  [],
		artists: {},
		albums:  {}
	};

	zlPlayer.playlist = {
		position: 0,
		track: null,
		length: 0,
		list: []
	};

	zlPlayer.emit = function() {
		var ev   = arguments[0];
		if (ev in zlPlayer.listeners) {
			var args = [];
			for (var i = 1; i < arguments.length; i++) {
				args.push(arguments[i]);
			}
			zlPlayer.listeners[ev].forEach(function(cb) {
				setTimeout(function() { $rootScope.$apply(cb.apply(cb, args)); }, 0);
			});
		}
	}

	zlPlayer.on = function(ev, cb) {
		if (ev in zlPlayer.listeners) {
			zlPlayer.listeners[ev].push(cb);
		}
	}

	zlPlayer.trackTime = function(time) {

		var tsec = Math.round(time/1000);
		var min  = Math.floor(tsec/60);
		var sec  = tsec % 60;

		var tstr = '';
		if (min < 10) {
			tstr = '0';
		}
		tstr += min + ':';
		if (sec < 10) {
			tstr += '0';
		}
		tstr += sec;

		return tstr;
	}

	zlPlayer.trackProgress = function(position, duration) {
		var tsec = Math.round(position/1000);
		/* prevent too many Angular.js scope refreshes */
		if (zlPlayer.currPos !== tsec) {
			zlPlayer.currPos = tsec;
			zlPlayer.emit('progress', zlPlayer.trackTime(position), zlPlayer.trackTime(duration));
		}
	}

	zlPlayer.onLoadSound = function(play) {
		console.log('zlPlayer.onLoadSound - emit load and progress event.');
		zlPlayer.emit('load', zlPlayer.playlist.track);
		zlPlayer.emit('progress', zlPlayer.trackTime(zlPlayer.sound.position), zlPlayer.trackTime(zlPlayer.sound.duration));
		if (play) {
			zlPlayer.play();
		}
	}

	zlPlayer.onEndTrack = function() {
		zlPlayer.emit('end');
		zlPlayer.playlist.position++;
		if (zlPlayer.playlist.position >= zlPlayer.playlist.length) {
			zlPlayer.playlist.position = 0;
			zlPlayer.stop();
		}
		else {
			zlPlayer.prepare(zlPlayer.playlist.position, true);
		}
	}

	zlPlayer.select = function(playlist, index) {
		if (index >= 0 && index < playlist.length) {
			zlPlayer.playlist.length = playlist.length;
			zlPlayer.playlist.list   = playlist;
			zlPlayer.prepare(index);
			return zlPlayer.playlist.list;
		}
		else {
			console.log('zlPlayer.select - error, index is out of range: ' + index + ' (playlist length: ' + playlist.length + ').');
			return [];
		}
	}

	zlPlayer.prepare = function(index, play) {
		if ( typeof index === 'undefined' && zlPlayer.sound != null ) {
			console.log('prepare index: nothing to prepare, already loaded.');
		}
		else {
			zlPlayer.stop();
			zlPlayer.playlist.position = index;
			zlPlayer.playlist.track    = zlPlayer.playlist.list[index];
			zlPlayer.sound = zlPlayer.sm2.createSound({ 
				url:          '/tracks/' + zlPlayer.playlist.track.id + '/audio',
				autoLoad:     true,
				autoPlay:     false,
				onload:       function() { zlPlayer.onLoadSound(play); },
				onplay:       function() { zlPlayer.emit('play', zlPlayer.playlist.track); },
				onpause:      function() { zlPlayer.emit('pause'); },
				onresume:     function() { zlPlayer.emit('play', zlPlayer.playlist.track); },
				onstop:       function() { zlPlayer.emit('stop'); },
				whileplaying: function() { zlPlayer.trackProgress(this.position, this.duration); },
				onfinish:     function() { zlPlayer.onEndTrack(); }
			});
		}
	}

	zlPlayer.play = function() {
		if (zlPlayer.sound !== null) {
			if (zlPlayer.sound.paused) {
				zlPlayer.sound.resume();
			}
			else {
				zlPlayer.sound.play();
			}
			zlPlayer.playlist.track.playing = true;
		}
	}

	zlPlayer.stop = function() {
		if (zlPlayer.sound != null) {
			zlPlayer.playlist.track.playing = false;
			zlPlayer.sound.stop();
			zlPlayer.sound.unload();
			zlPlayer.sound = null;
		}
	}

	zlPlayer.pause = function() {
		if (zlPlayer.sound != null) {
			zlPlayer.sound.pause();
		}
	}

	zlPlayer.load = function(tracklist) {

		/* stop sound */
		zlPlayer.stop();

		zlPlayer.library = {
			tracks:  [],
			artists: {},
			albums:  {}
		};

		zlPlayer.playlist = {
			position: 0,
			track: null,
			length: 0,
			list: []
		};

		/* build library */
		tracklist.forEach(function(track) {
			zlPlayer.library.tracks[track.id] = track;
			var artist = track.artist;
			var album  = track.album;
			if (!(album in zlPlayer.library.albums)) {
				zlPlayer.library.albums[album] = {
					title: album,
					artist: artist,
					tracks: []
				};
			}
			if (!(artist in zlPlayer.library.artists)) {
				zlPlayer.library.artists[artist] = {
					name: artist,
					albums: {},
					tracks: []
				};
			}
			if (!(album in zlPlayer.library.artists[artist].albums)) {
				zlPlayer.library.artists[artist].albums[album] = {
					title: album,
					tracks: []
				};
			}
			zlPlayer.library.artists[artist].albums[album].tracks.push(track);
			zlPlayer.library.artists[artist].tracks.push(track);
			zlPlayer.library.albums[album].tracks.push(track);
		});

		/* notify listeners */
		zlPlayer.emit('library');
	}

	return zlPlayer;
};
angular.module('zillout').factory('zlPlayer', zlPlayer);
