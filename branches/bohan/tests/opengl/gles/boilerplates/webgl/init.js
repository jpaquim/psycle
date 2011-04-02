
function log(obj) { console.log(obj) }

gl = {};

gl.getCtx = function(canvas, attr) {
	log('gl.getCtx: for canvas id: ' + canvas.id);
	function getCtx(name) { return attr === undefined ? canvas.getContext(name) : canvas.getContext(name, attr); }
	var ctx = getCtx('webgl') || getCtx('experimental-webgl');
	if(!ctx) throw 'Your browser does not support webgl';
	return ctx;
}

gl.resizeCanvas = function(canvas, width, height) {
	if(width === undefined) width = canvas.scrollWidth;
	if(height === undefined) height = canvas.scrollHeight;
	log('gl.resizeCanvas: ' + width + ', ' + height);
	canvas.width = width;
	canvas.height = height;
}
