function zlArtistsCtrl($scope, zlId3, zlPlayer, zlRouter) {

	var zlArtistsCtrl = {};

	console.log('zlArtistsCtrl invoked.');

	zlArtistsCtrl.update = function() {

		var artists = {
			view:   'all',
			name:   '',
			list:   [],
			albums: [],
			tracks: []
		};

		if (zlRouter.params.name) {
			artists.name = zlRouter.params.name;
			if (zlRouter.params.what === 'tracks') {
				artists.view   = 'tracks';
				artists.tracks = zlPlayer.artistTracks(artists.name);
			}
			else {
				artists.view   = 'artist';
				artists.albums = zlPlayer.albums(artists.name);
			}
		}
		else {
			artists.list = zlPlayer.artists();
		}
		
		$scope.artists = artists;
		console.log('zlArtistsCtrl.update - view: ' + $scope.artists.view + ', name: ' + $scope.artists.name);
		console.log('zlArtistsCtrl.update - list has ' + $scope.artists.list.length + ' items');
		console.log('zlArtistsCtrl.update - albums has ' + $scope.artists.albums.length + ' items');
	}

	zlArtistsCtrl.update();

	zlArtistsCtrl.onLoadLibrary = function() {
		console.log('zlArtistsCtrl.onLoadLibrary - get artist list.');
		zlArtistsCtrl.update();
	}
	zlPlayer.on('library', zlArtistsCtrl.onLoadLibrary);

	$scope.show = function(name) {
		console.log('show artist:' + name);
		zlRouter.path('/artists/' + name);
	}

	$scope.showAlbum = function(artist, album) {
		zlRouter.path('/albums/' + artist + '/' + album);
	}

	$scope.showTracks = function(name) {
		zlRouter.path('/artists/' + name + '/tracks');
	}

	$scope.play = function(index) {
		zlPlayer.select($scope.artists.tracks, index, { title: $scope.artists.name + ' (all songs)' });
		zlPlayer.prepare(index, true);
		zlRouter.path('/playlist');
	}

	$scope.shuffle = function() {
		var shlist = zlPlayer.shuffle($scope.artists.tracks);
		zlPlayer.select(shlist, 0 , { title: $scope.artists.name + ' (all songs)' });
		zlPlayer.prepare(0, true);
		zlRouter.path('/playlist');
	}

	return zlArtistsCtrl;
}
angular.module('zillout').controller('zlArtistsCtrl', zlArtistsCtrl);
