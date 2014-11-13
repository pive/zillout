function zlDeckCtrl($scope, zlId3, zlPlayer) {

	var zlDeckCtrl = {};

	console.log('zlDeckCtrl invoked.');

	$scope.deck = {
		playing: false,
		visible: false,
		title: '',
		artist: '',
		album: '',
		time: '00:00',
		duration: '',
		imagesrc: null
	};

	$scope.stop = function() {
		zlPlayer.stop();
	}

	$scope.toggle = function() {
		if ($scope.deck.playing) {
			zlPlayer.pause();
		}
		else {
			zlPlayer.play();
		}
	}

	zlDeckCtrl.onPlay = function(track) {
		$scope.deck.playing = true;
	}

	zlDeckCtrl.onStop = function() {
		$scope.deck.playing = false;
	}

	zlDeckCtrl.onPause = function() {
		$scope.deck.playing = false;
	}

	zlDeckCtrl.onLoad = function(track) {
		$scope.deck.imagesrc = '/tracks/' + track.id + '/image';
		$scope.deck.title  = track.title;
		$scope.deck.artist = track.artist;
		$scope.deck.album  = track.album;
	}

	zlDeckCtrl.onProgress = function(time, duration) {
		$scope.deck.time = time + ' / ' + duration;
	}

	zlPlayer.on('load',     zlDeckCtrl.onLoad);
	zlPlayer.on('play',     zlDeckCtrl.onPlay);
	zlPlayer.on('stop',     zlDeckCtrl.onStop);
	zlPlayer.on('pause',    zlDeckCtrl.onPause);
	zlPlayer.on('progress', zlDeckCtrl.onProgress);

	return zlDeckCtrl;
}
angular.module('zillout').controller('zlDeckCtrl', zlDeckCtrl);
