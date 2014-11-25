function zlPlayer($rootScope, zlEmitter, zlId3) {

	var zlPlayer = zlEmitter.new([
		'library',
		'load',
		'play',
		'pause',
		'stop',
		'end',
		'progress'
		]);

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

	zlPlayer.sound   = null;
	zlPlayer.library = {
		tracks:  [],
		artists: {},
		albums:  {}
	};

	zlPlayer.playlist = {
		title: 'All songs',
		imgsrc: '',
		position: 0,
		track: null,
		length: 0,
		list: []
	};

	console.log('zlPlayer invoked.');

	zlPlayer.onId3Tracklist = function(path, tracks) {
		console.log('zlPlayer.onId3Library - received id3 tracklist event, calling load, library path: ' + path);
		zlPlayer.load(tracks);
	}
	zlId3.on('tracklist', zlPlayer.onId3Tracklist);

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

	zlPlayer.shuffle = function(array) {
		var top, index, elem, list = [];
		if (array.length) {
			list = array.slice(0);
			top  = list.length;
			while (top)
			{
				index = Math.floor(Math.random() * top);
				top--;
				elem = list[index];
				list[index] = list[top];
				list[top]   = elem;
			}
		}
		return list;
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

	zlPlayer.select = function(playlist, index, options) {

		if (playlist.length === 0) {
			console.log('zlPlayer.select - playlist is empty.');
			return [];
		}
		
		zlPlayer.playlist.title  = 'All songs';
		zlPlayer.playlist.imgsrc = '';

		if (options && options.title) {
			zlPlayer.playlist.title  = options.title;
		}
		if (options && options.imgsrc) {
			zlPlayer.playlist.imgsrc  = options.imgsrc;
		}

		if (index >= 0 && index < playlist.length) {
			zlPlayer.playlist.length = playlist.length;
			zlPlayer.playlist.list   = playlist;
			zlPlayer.prepare(index);
			return zlPlayer.playlist.list;
		}
		
		console.log('zlPlayer.select - error, index is out of range: ' + index + ' (playlist length: ' + playlist.length + ').');
		return [];
	}

	zlPlayer.current = function() {
		if (zlPlayer.playlist.length > 0) {
			zlPlayer.prepare();
		}
		return zlPlayer.playlist.list;
	}

	zlPlayer.title = function() {
		return zlPlayer.playlist.title;
	}

	zlPlayer.imgsrc = function() {
		return zlPlayer.playlist.imgsrc;	
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

	zlPlayer.previous = function() {
		if (zlPlayer.sound && zlPlayer.playlist.track.playing) {
			var threshold = 4;
			if (zlPlayer.sound.position < (1000 * threshold)) {
				console.log('zlPlayer.previous - sound playing position under ' + threshold + ' sec, rewind to previous track.');
				zlPlayer.playlist.position--;
			}
			else {
				console.log('zlPlayer.previous - sound playing position over ' + threshold + ' sec, rewind to current track start.');
			}
		}
		else {
			console.log('zlPlayer.previous - no sound playing, rewind to previous track.');
			zlPlayer.playlist.position--;
		}
		if (zlPlayer.playlist.position < 0) {
			console.log('zlPlayer.previous - reached beginning of playlist, set to last track.');
			zlPlayer.playlist.position = zlPlayer.playlist.length - 1;
		}
		zlPlayer.prepare(zlPlayer.playlist.position, zlPlayer.playlist.track.playing);
	}

	zlPlayer.next = function() {
		zlPlayer.playlist.position++;
		if (zlPlayer.playlist.position >= zlPlayer.playlist.length) {
			console.log('zlPlayer.previous - reached end of playlist, reset to index 0.');
			zlPlayer.playlist.position = 0;
		}
		zlPlayer.prepare(zlPlayer.playlist.position, zlPlayer.playlist.track.playing);
	}

	zlPlayer.artists = function() {
		var list = [];
		for (var name in zlPlayer.library.artists) {
			list.push(zlPlayer.library.artists[name]);
		}
		/* sort artists by name */
		list.sort(function(a, b) {
			var la = a.name?a.name.toLowerCase():'';
			var lb = b.name?b.name.toLowerCase():'';
			if (la > lb) {
				return 1;
			}
			if (la < lb) {
				return -1;
			}
			return 0;
		});
		return list;
	}

	zlPlayer.albums = function(name) {
		var list = [];
		var dict = zlPlayer.library.albums;
		if (name) {
			if (!zlPlayer.library.artists[name]) {
				console.log('no albums for artist: ' + name);
				return [];
			}
			dict = zlPlayer.library.artists[name].albums;
		}
		for (var name in dict) {
			list.push(dict[name]);
		}
		/* sort albums by title */
		list.sort(function(a, b) {
			var la = a.title?a.title.toLowerCase():'';
			var lb = b.title?b.title.toLowerCase():'';
			if (la > lb) {
				return 1;
			}
			if (la < lb) {
				return -1;
			}
			return 0;
		});
		return list;
	}

	zlPlayer.artistTracks = function(name) {
		if (!zlPlayer.library.artists[name]) {
			console.log('no albums for artist: ' + name);
			return [];
		}
		var list = zlPlayer.library.artists[name].tracks.slice(0);
		/* sort tracks by title */
		list.sort(function(a, b) {
			var la = a.title?a.title.toLowerCase():(a.filename?a.filename.toLowerCase():'');
			var lb = b.title?b.title.toLowerCase():(b.filename?b.filename.toLowerCase():'');
			if (la > lb) {
				return 1;
			}
			if (la < lb) {
				return -1;
			}
			return 0;
		});
		return list;
	}

	zlPlayer.albumTracks = function(artist, title) {

		if (!zlPlayer.library.artists[artist]) {
			console.log('no albums for artist: ' + name);
			return [];
		}
		if (!zlPlayer.library.artists[artist].albums[title]) {
			console.log('no tracks for album: ' + title);
			return [];
		}
		return zlPlayer.library.artists[artist].albums[title].tracks.slice(0);
	}

	zlPlayer.allTracks = function() {
		var list = zlPlayer.library.tracks.slice(0);
		/* sort tracks by title */
		list.sort(function(a, b) {
			var la = a.title?a.title.toLowerCase():(a.filename?a.filename.toLowerCase():'');
			var lb = b.title?b.title.toLowerCase():(b.filename?b.filename.toLowerCase():'');
			if (la > lb) {
				return 1;
			}
			if (la < lb) {
				return -1;
			}
			return 0;
		});
		return list;
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
			if (typeof track.title === 'undefined') {
				track.title = track.filename;
			}
			if (typeof track.artist === 'undefined') {
				track.artist = '[unknown]';
			}
			if (typeof track.album === 'undefined') {
				track.album = '[unknown]';
			}
			zlPlayer.library.tracks.push(track);
			var artist = track.artist;
			var album  = track.album;
			if (!(album in zlPlayer.library.albums)) {
				zlPlayer.library.albums[album] = {
					title:  album,
					artist: artist,
					imgsrc: '/albums/' + artist + '/' + album + '/image',
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
					imgsrc: '/albums/' + artist + '/' + album + '/image',
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
