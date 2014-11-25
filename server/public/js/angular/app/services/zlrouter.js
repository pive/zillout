function zlRouter($rootScope, $location, $routeParams, $window, zlEmitter) {

	var zlRouter = zlEmitter.new([ 'location' ]);

	zlRouter.location = $location.path();
	zlRouter.params   = $routeParams;

	console.log('zlRouter invoked, location: ' + zlRouter.location);

	$rootScope.$on('$routeChangeSuccess', function (event, current, previous) {
		if (current.originalPath) {
			zlRouter.location = current.originalPath;
			zlRouter.params   = $routeParams;
			console.log('$rootScope on $routeChangeSuccess, current: ' + current.originalPath);
			console.log('$rootScope on $routeChangeSuccess, zlRouter emitting location event.');
			zlRouter.emit('location', current.originalPath);
		}
	});

	zlRouter.path = function(path) {
		$location.path(path);
	}

	zlRouter.back = function() {
		$window.history.back();
	}

	return zlRouter;
}
angular.module('zillout').factory('zlRouter', zlRouter);
