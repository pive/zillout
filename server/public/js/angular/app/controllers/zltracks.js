function zlTracksCtrl($scope, zlId3, zlPlayer) {

	var zlTracksCtrl = {};

	$scope.alltracks = [];

	console.log('zlTracksCtrl invoked.');

	zlTracksCtrl.onLoadLibrary = function() {
		console.log('zlTracksCtrl.onLoadLibrary - set tracks and prepare sound.');
		$scope.alltracks = zlPlayer.select(zlPlayer.library.tracks, 0);
	}
	zlPlayer.on('library', zlTracksCtrl.onLoadLibrary);

	$scope.play = function(index) {
		zlPlayer.prepare(index, true);
	}

	return zlTracksCtrl;
}
angular.module('zillout').controller('zlTracksCtrl', zlTracksCtrl);
