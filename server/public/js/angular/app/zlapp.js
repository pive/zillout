
/* 
 * ----------------------------------------------------------------------------
 * 	Application definition and configuration
 * 
 * ----------------------------------------------------------------------------
 */

/* Declare application */
angular.module('zillout', [ 'ngRoute', 'ui.bootstrap' ]);

/* Application Router */
function _zl_f_route($routeProvider) {
	$routeProvider.when('/tracks', {
		templateUrl: 'views/tracks.html',
		controller: 'zlTracksCtrl'
	});
	$routeProvider.when('/settings', {
		templateUrl: 'views/settings.html',
		controller: 'zlSettingsCtrl'
	});
	/*$routeProvider.when('/artists/:artistname', {
		templateUrl: 'views/main.html',
		controller: 'zlMainCtrl'
	});*/
	/* default to root */
	$routeProvider.otherwise({ redirectTo: '/tracks' });
};
angular.module('zillout').config(['$routeProvider', _zl_f_route]);
