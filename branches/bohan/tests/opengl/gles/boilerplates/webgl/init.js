
var bohan = bohan || {};

bohan.log = bohan.log || {};

bohan.log.log = function(obj) { console.log(obj); }

bohan.ajax = bohan.ajax || {};

bohan.ajax.readText = function(url) {
	var req = new XMLHttpRequest();
	req.open('get', url, false);
	req.send();
	if(req.status !== 200 /* OK */) throw req.status;
	return req.responseText;
}

bohan.gl = bohan.gl || {};

bohan.gl.getCtx = function(canvas, attr) {
	bohan.log.log('gl.getCtx: for canvas id: ' + canvas.id);
	function getCtx(name) { return attr === undefined ? canvas.getContext(name) : canvas.getContext(name, attr); }
	var ctx = getCtx('webgl') || getCtx('experimental-webgl');
	if(!ctx) throw 'Your browser does not support webgl';
	return ctx;
}

bohan.gl.resizeCanvas = function(canvas, width, height) {
	if(width === undefined) width = canvas.scrollWidth;
	if(height === undefined) height = canvas.scrollHeight;
	bohan.log.log('gl.resizeCanvas: ' + width + ', ' + height);
	canvas.width = width;
	canvas.height = height;
}
