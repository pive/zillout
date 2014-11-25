function zlPlaylistCtrl($scope, zlId3, zlPlayer) {

	var zlPlaylistCtrl = {};

	$scope.playlist = {
		tracks: zlPlayer.current(),
		title: zlPlayer.title(),
		imgsrc: zlPlayer.imgsrc()
	}

	if (zlPlayer.sound === null) {
		$scope.playlist.tracks = zlPlayer.select(zlPlayer.allTracks(), 0);
		console.log('zlPlaylistCtrl invoked, got ' + $scope.playlist.tracks.length + ' tracks from zlPlayer.');
	}
	else {
		console.log('zlPlaylistCtrl invoked, zlPlayer already loaded.');
	}

	zlPlaylistCtrl.onLoadLibrary = function() {
		console.log('zlPlaylistCtrl.onLoadLibrary - set tracks and prepare sound.');
		$scope.playlist.tracks = zlPlayer.select(zlPlayer.allTracks(), 0);
	}
	zlPlayer.on('library', zlPlaylistCtrl.onLoadLibrary);

	$scope.play = function(index) {
		zlPlayer.prepare(index, true);
	}

	return zlPlaylistCtrl;
}
angular.module('zillout').controller('zlPlaylistCtrl', zlPlaylistCtrl);
