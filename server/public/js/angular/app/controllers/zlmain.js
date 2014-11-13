function zlMainCtrl($scope, zlId3, zlPlayer, zlRouter) {

	var zlMainCtrl = {};

	console.log('zlMainCtrl invoked.');

	$scope.status   = 'Booting... [zlId3.status = ' + JSON.stringify(zlId3.status) + ']';
	$scope.progress = {
		count: 0,
		total: 1000
	};
	$scope.display = {
		progress: false,
		deck:     false,
		nav:      false,
		tracks:   false
	};
	$scope.spy = {
		path: ''
	};

	zlMainCtrl.displayProgress = function() {
		console.log('displaying progress.');
		$scope.display = {
			progress: true,
			deck:     false,
			nav:      false,
			tracks:   false
		}
	}

	zlMainCtrl.displayTracks = function() {
		console.log('displaying tracks.');
		$scope.display = {
			progress: false,
			deck:     true,
			nav:      true,
			tracks:   true
		}
	}

	/* zlid3 init and handlers */
	zlMainCtrl.onID3Load = function(path) {
		console.log('Library started loading: ' + path);
		zlPlayer.stop();
		zlMainCtrl.displayProgress();
		$scope.status = 'Library is loading ' + path + '...';
	}

	zlMainCtrl.onID3Progress = function(progress) {
		if (!$scope.display.progress) {
			zlMainCtrl.displayProgress();
		}
		$scope.progress.count  = Math.round((progress.count * $scope.progress.total) / progress.total);
		$scope.status = 'Library is loading [' + progress.count + '/' + progress.total + ']...';
	}

	zlMainCtrl.onID3Loaded = function(path) {
		console.log('Library finished loading: ' + path);
		$scope.display.progress = false;
		$scope.status = 'Library has loaded ' + path + '.';
	}

	zlMainCtrl.onID3Error = function(err) {
		console.log('Library failed loading: ' + err);
		$scope.display.progress = false;
	}

	zlMainCtrl.onID3Tracks = function(path, tracks) {
		console.log('Library sent track list for ' + path + '(' + tracks.length + ')');
		zlPlayer.load(tracks);
	}

	zlId3.on('load', zlMainCtrl.onID3Load);
	zlId3.on('loading', zlMainCtrl.onID3Progress);
	zlId3.on('loaded', zlMainCtrl.onID3Loaded);
	zlId3.on('loaderr', zlMainCtrl.onID3Error);
	zlId3.on('tracklist', zlMainCtrl.onID3Tracks);
	zlId3.load();

	/* zlPlayer init and handlers */
	zlMainCtrl.onPlaylistChange = function() {
		console.log('Playlist change event.');
		if (zlRouter.location !== '/settings') {
			zlMainCtrl.displayTracks();
		}
	}
	zlPlayer.on('playlist', zlMainCtrl.onPlaylistChange);

	/* zlrouter init and handlers */
	zlMainCtrl.onLocationChange = function() {
		var path = zlRouter.location;
		$scope.spy.path = path;
		if (path === '/settings') {
			$scope.display = {
				progress: false,
				deck:     true,
				nav:      false,
				tracks:   false
			}
		}
		else {
			zlMainCtrl.displayTracks();
		}
	}
	zlRouter.on('location', zlMainCtrl.onLocationChange);
	/* call it once at init */
	zlMainCtrl.onLocationChange();

	/*zlMainCtrl.onLoadTrack = function() {
		$scope.status = 'Track loaded.';
	}

	zlMainCtrl.onPlayTrack = function(index) {
		$scope.status = 'Track playing.';
		$scope.playindex = index;
		$scope.library.tracks[$scope.playindex].playing = true;
	}

	zlMainCtrl.onStopTrack = function() {
		console.log('zlMainCtrl.onStopTrack: ' + $scope.playindex);
		$scope.library.tracks[$scope.playindex].playing = false;
	}

	zlMainCtrl.onEndTrack = function() {
		$scope.library.tracks[$scope.playindex].playing = false;
		$scope.playindex++;
		$scope.playAllTracks($scope.playindex);
	}

	$scope.playAllTracks = function(index) {
		zlPlayer.play(index);
	}

	zlPlayer.on('load', zlMainCtrl.onLoadTrack);
	zlPlayer.on('play', zlMainCtrl.onPlayTrack);
	zlPlayer.on('stop', zlMainCtrl.onStopTrack);
	zlPlayer.on('end', zlMainCtrl.onEndTrack);*/

	return zlMainCtrl;
};
angular.module('zillout').controller('zlMainCtrl', zlMainCtrl);
