function zlSettingsCtrl($scope, zlRouter, zlId3) {

	var zlSettingsCtrl = {};

	console.log('zlSettingsCtrl invoked.');

	$scope.settings = {
		path: zlId3.status.path
	};

	zlSettingsCtrl.onID3Loaded = function(path) {
		console.log('zlSettingsCtrl.onID3Loaded - path: ' + path);
		$scope.settings.path = path;
	}
	zlId3.on('loaded', zlSettingsCtrl.onID3Loaded);

	$scope.reload = function() {
		zlId3.load($scope.settings.path);
		zlRouter.path('#/');
	}

	$scope.close = function() {
		zlRouter.back();
	}

	return zlSettingsCtrl;
}
angular.module('zillout').controller('zlSettingsCtrl', zlSettingsCtrl);
