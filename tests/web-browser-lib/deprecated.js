var bohan = bohan || {};

bohan.log = function(/*arguments*/) {
	console.log(arguments);
};
	
bohan.deprecationEnabled = true;
	
bohan.deprecated = function(f, replacement) {
	if(!bohan.deprecationEnabled) return f;
	return function(/*arguments*/) {
		var msg = 'warning: deprecated function \'' + f.toString().split('{')[0].replace(/ $/, '') + '\' called.';
		if(replacement) msg += ' Please use \'' + replacement + '\' instead.';
		bohan.log(msg);
		return f.apply(this, arguments);
	};
}

