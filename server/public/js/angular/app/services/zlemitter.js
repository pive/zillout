function zlEmitter($rootScope) {

	var zlEmitter = {};

	console.log('zlEmitter invoked.');

	zlEmitter.new = function(events) {

		var _CEventEmitter = {
			scope: $rootScope,
			listeners: {}
		};

		events.forEach(function(name) {
			_CEventEmitter.listeners[name] = [];
		});
		
		_CEventEmitter.emit = function() {
			var ev = arguments[0];
			if (ev in _CEventEmitter.listeners) {
				var args = [];
				for (var i = 1; i < arguments.length; i++) {
					args.push(arguments[i]);
				}
				_CEventEmitter.listeners[ev].forEach(function(cb) {
					setTimeout(function() { _CEventEmitter.scope.$apply(cb.apply(cb, args)); }, 0);
				});
			}
		}

		_CEventEmitter.on = function(ev, cb) {
			if (ev in _CEventEmitter.listeners) {
				_CEventEmitter.listeners[ev].push(cb);
			}
		}

		_CEventEmitter.removeListeners = function(ev) {
			if (ev) {
				if (ev in _CEventEmitter.listeners) {
					_CEventEmitter.listeners[ev] = [];
				}
			}
			else {
				for (var ev in _CEventEmitter.listeners) {
					_CEventEmitter.listeners[ev] = [];
				}
			}
		}

		return _CEventEmitter;
	};

	return zlEmitter;
}
angular.module('zillout').factory('zlEmitter', zlEmitter);
