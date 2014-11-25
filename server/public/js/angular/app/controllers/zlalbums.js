function zlAlbumsCtrl($scope, zlId3, zlPlayer, zlRouter) {

	var zlAlbumsCtrl = {};

	console.log('zlAlbumsCtrl invoked.');

	zlAlbumsCtrl.update = function() {

		var albums = {
			view:   'all',
			artist: '',
			name:   '',
			list:   [],
			tracks: []
		};
		
		if (zlRouter.params.artist && zlRouter.params.title) {

			albums.artist = zlRouter.params.artist;
			albums.name   = zlRouter.params.title;
			albums.view   = 'album';
			albums.tracks = zlPlayer.albumTracks(albums.artist, albums.name);
			albums.imgsrc = '/albums/' + albums.artist + '/' + albums.name + '/image';
		}
		else {
			albums.list = zlPlayer.albums();
		}

		$scope.albums = albums;
	}

	zlAlbumsCtrl.update();

	zlAlbumsCtrl.onLoadLibrary = function() {
		console.log('zlAlbumsCtrl.onLoadLibrary - get albums list.');
		zlAlbumsCtrl.update();
	}
	zlPlayer.on('library', zlAlbumsCtrl.onLoadLibrary);

	$scope.show = function(artist, name) {
		zlRouter.path('/albums/' + artist + '/' + name);
	}

	$scope.play = function(index) {
		var options = {
			title:  $scope.albums.name,
			imgsrc: '/albums/' + $scope.albums.artist + '/' + $scope.albums.name + '/image'
		}
		zlPlayer.select($scope.albums.tracks, index ,options);
		zlPlayer.prepare(index, true);
		zlRouter.path('/playlist');
	}

	$scope.shuffle = function() {
		var options = {
			title:  $scope.albums.name,
			imgsrc: '/albums/' + $scope.albums.artist + '/' + $scope.albums.name + '/image'
		}
		var shlist = zlPlayer.shuffle($scope.albums.tracks);
		zlPlayer.select(shlist, 0 ,options);
		zlPlayer.prepare(0, true);
		zlRouter.path('/playlist');
	}

	return zlAlbumsCtrl;
}
angular.module('zillout').controller('zlAlbumsCtrl', zlAlbumsCtrl);
