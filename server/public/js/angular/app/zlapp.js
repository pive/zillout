
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
	$routeProvider.when('/', {
		templateUrl: 'views/tracks.html',
		controller: 'zlTracksCtrl'
	});
	$routeProvider.when('/playlist', {
		templateUrl: 'views/playlist.html',
		controller: 'zlPlaylistCtrl'
	});
	$routeProvider.when('/artists', {
		templateUrl: 'views/artists.html',
		controller: 'zlArtistsCtrl'
	});
	$routeProvider.when('/artists/:name', {
		templateUrl: 'views/artists.html',
		controller: 'zlArtistsCtrl'
	});
	$routeProvider.when('/artists/:name/:what', {
		templateUrl: 'views/artists.html',
		controller: 'zlArtistsCtrl'
	});
	$routeProvider.when('/artists/:name', {
		templateUrl: 'views/artists.html',
		controller: 'zlArtistsCtrl'
	});
	$routeProvider.when('/albums', {
		templateUrl: 'views/albums.html',
		controller: 'zlAlbumsCtrl'
	});
	$routeProvider.when('/albums/:artist/:title', {
		templateUrl: 'views/albums.html',
		controller: 'zlAlbumsCtrl'
	});
	$routeProvider.when('/settings', {
		templateUrl: 'views/settings.html',
		controller: 'zlSettingsCtrl'
	});
	/* default to root */
	$routeProvider.otherwise({ redirectTo: '/' });
};
angular.module('zillout').config(['$routeProvider', _zl_f_route]);
