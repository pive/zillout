function zlRouter($rootScope, $location) {

	var zlRouter = {};

	zlRouter.listeners = {
		location: []
	};
	zlRouter.location = $location.path();

	console.log('zlRouter invoked, location: ' + zlRouter.location);

	zlRouter.emit = function() {
		var ev   = arguments[0];
		if (ev in zlRouter.listeners) {
			var args = [];
			for (var i = 1; i < arguments.length; i++) {
				args.push(arguments[i]);
			}
			zlRouter.listeners[ev].forEach(function(cb) {
				setTimeout(function() { $rootScope.$apply(cb.apply(cb, args)); }, 0);
			});
		}
	}

	zlRouter.on = function(ev, cb) {
		if (ev in zlRouter.listeners) {
			zlRouter.listeners[ev].push(cb);
		}
	}

	$rootScope.$on('$routeChangeSuccess', function (event, current, previous) {
		zlRouter.location = current.originalPath;
		zlRouter.emit('location', current.originalPath);
	});

	return zlRouter;
}
angular.module('zillout').factory('zlRouter', zlRouter);
