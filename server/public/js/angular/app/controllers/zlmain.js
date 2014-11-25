function zlMainCtrl($scope, zlId3, zlRouter) {

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
	$scope.nav = {
		songs:    true,
		artists:  false,
		playlist: false,
		albums:   false
	};

	zlMainCtrl.navStatus = function(path) {
		var map = {
			'/artists': 'artists',
			'/playlist': 'playlist',
			'/albums': 'albums',
		};

		$scope.nav = {
			songs:    false,
			artists:  false,
			playlist: false,
			albums:   false
		};

		var active = 'songs';
		for (var base in map) {
			if (path.substr(0, base.length) === base) {
				active = map[base];
				break;
			}
		}

		console.log('zlMainCtrl.navStatus - active: ' + active);
		$scope.nav[active] = true;
	}

	zlMainCtrl.displayProgress = function() {
		$scope.display = {
			progress: true,
			deck:     false,
			nav:      false,
			tracks:   false
		}
		console.log('zlMainCtrl.displayProgress - displaying progress, display: ' + JSON.stringify($scope.display));
	}

	zlMainCtrl.displayTracks = function() {
		$scope.display = {
			progress: false,
			deck:     true,
			nav:      true,
			tracks:   true
		}
		console.log('zlMainCtrl.displayTracks - displaying tracks, display: ' + JSON.stringify($scope.display));
	}

	/* zlid3 init and handlers */
	zlMainCtrl.onID3Load = function(path) {
		console.log('zlMainCtrl.onID3Load - Library started loading: ' + path);
		zlMainCtrl.displayProgress();
		$scope.status = 'Library is loading ' + path + '...';
	}

	zlMainCtrl.onID3Progress = function(progress) {
		if (!$scope.display.progress) {
			console.log('zlMainCtrl.onID3Progress - progress not displayed, switch to progress display.');
			zlMainCtrl.displayProgress();
		}
		$scope.progress.count  = Math.round((progress.count * $scope.progress.total) / progress.total);
		$scope.status = 'Library is loading [' + progress.count + '/' + progress.total + ']...';
	}

	zlMainCtrl.onID3Loaded = function(path) {
		console.log('zlMainCtrl.onID3Loaded - Library finished loading: ' + path);
		$scope.status = 'Library has loaded ' + path + '.';
		zlMainCtrl.displayTracks();
	}

	zlMainCtrl.onID3Error = function(err) {
		console.log('Library failed loading: ' + err);
		$scope.display.progress = false;
	}

	zlId3.on('load',    zlMainCtrl.onID3Load);
	zlId3.on('loading', zlMainCtrl.onID3Progress);
	zlId3.on('loaded',  zlMainCtrl.onID3Loaded);
	zlId3.on('loaderr', zlMainCtrl.onID3Error);

	/* THIS IS NEVER CALLED AT THIS TIME
	zlMainCtrl.onPlaylistChange = function() {
		console.log('Playlist change event.');
		if (zlRouter.location !== '/settings') {
			zlMainCtrl.displayTracks();
		}
		console.log('zlMainCtrl.onPlaylistChange - display: ' + JSON.stringify($scope.display));
	}
	zlPlayer.on('playlist', zlMainCtrl.onPlaylistChange);*/

	/* zlrouter init and handlers */
	zlMainCtrl.onLocationChange = function() {

		var path  = zlRouter.location;

		if (path === '/settings') {
			$scope.display = {
				progress: false,
				deck:     $scope.display.deck,
				nav:      false,
				tracks:   false
			}
		}
		else {
			zlMainCtrl.displayTracks();
			zlMainCtrl.navStatus(path);
		}
		console.log('zlMainCtrl.onLocationChange - display: ' + JSON.stringify($scope.display));
	}
	zlRouter.on('location', zlMainCtrl.onLocationChange);
	/* call it once at init */
	zlMainCtrl.onLocationChange();

	return zlMainCtrl;
};
angular.module('zillout').controller('zlMainCtrl', zlMainCtrl);
