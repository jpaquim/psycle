
gl = {};

gl.getCtx = function(canvas, attr) {
	function getCtx(name) { return attr === undefined ? canvas.getContext(name) : canvas.getContext(name, attr); }
	return getCtx('webgl') || getCtx('experimental-webgl');
}

