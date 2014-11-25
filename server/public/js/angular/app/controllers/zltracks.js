function zlTracksCtrl($scope, zlId3, zlPlayer, zlRouter) {

	var zlTracksCtrl = {};

	console.log('zlTracksCtrl invoked');

	$scope.alltracks = zlPlayer.allTracks();

	/*if (zlPlayer.sound === null) {
		$scope.alltracks = zlPlayer.select(zlPlayer.library.tracks, 'All songs', 0);
		console.log('zlTracksCtrl invoked, got ' + $scope.alltracks.length + ' tracks from zlPlayer.');
	}
	else {
		console.log('zlTracksCtrl invoked, zlPlayer already loaded.');
	}*/

	zlTracksCtrl.onLoadLibrary = function() {
		console.log('zlTracksCtrl.onLoadLibrary - set tracks and prepare sound.');
		$scope.alltracks = zlPlayer.allTracks();
	}
	zlPlayer.on('library', zlTracksCtrl.onLoadLibrary);

	$scope.play = function(index) {
		zlPlayer.select($scope.alltracks, index);
		zlPlayer.prepare(index, true);
		zlRouter.path('/playlist');
	}

	$scope.shuffle = function() {
		var shlist = zlPlayer.shuffle($scope.alltracks);
		zlPlayer.select(shlist, 0);
		zlPlayer.prepare(0, true);
		zlRouter.path('/playlist');
	}
	
	return zlTracksCtrl;
}
angular.module('zillout').controller('zlTracksCtrl', zlTracksCtrl);
