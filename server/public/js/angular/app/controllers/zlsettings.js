function zlSettingsCtrl($scope, $location, zlId3) {

	var zlSettingsCtrl = {};

	console.log('zlSettingsCtrl invoked.');

	$scope.settings = {
		path: zlId3.status.path
	};

	$scope.reload = function() {
		zlId3.load($scope.settings.path);
		$location.path('#/');
	}

	$scope.close = function() {
		$location.path('#/');
	}

	return zlSettingsCtrl;
}
angular.module('zillout').controller('zlSettingsCtrl', zlSettingsCtrl);
