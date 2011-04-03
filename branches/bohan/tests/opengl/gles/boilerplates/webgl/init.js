
var bohan = bohan || {};

bohan.log = bohan.log || {};

bohan.log.log = function(obj) { console.log(obj); }

bohan.ajax = bohan.ajax || {};

bohan.ajax.readText = function(url) {
	var req = new XMLHttpRequest();
	req.open('get', url, false);
	req.send();
	if(req.status !== 200 /* OK */) throw 'bohan.ajax.readText: status: ' + req.status;
	return req.responseText;
}

bohan.Gl = bohan.Gl || function(canvas, attr) {
	bohan.log.log('bohan.Gl: for canvas id: ' + canvas.id);
	this.canvas = canvas;
	function getCtx(name) { return attr === undefined ? canvas.getContext(name) : canvas.getContext(name, attr); }
	this.ctx = getCtx('webgl') || getCtx('experimental-webgl');
	if(!this.ctx) throw 'bohan.Gl: Your browser does not support webgl';
}

bohan.Gl.prototype.canvasResized = function() {
	var width = this.canvas.scrollWidth;
	var height = this.canvas.scrollHeight;
	bohan.log.log('bohan.Gl.prototype.canvasResized: ' + width + ', ' + height);
	this.canvas.width = width;
	this.canvas.height = height;
}

bohan.Gl.prototype._compileShader = function(type, text) {
	var shader = this.ctx.createShader(type);
	if(shader === 0) throw 'bohan.Gl.prototype._compileShader: could not create shader: type: ' + type;
	this.ctx.shaderSource(shader, text);
	this.ctx.compileShader(shader);
	var compiled = this.ctx.getShaderParameter(shader, this.ctx.COMPILE_STATUS);
	if(!compiled) {
		var msg = this.ctx.getShaderInfoLog(shader);
		this.ctx.deleteShader(shader);
		throw('bohan.Gl.prototype._compileShader: could not compile shader: ' + msg);
	}
	return shader;
}

bohan.Gl.prototype.linkProgram = function(vertexShaderUrl, fragmentShaderUrl) {
	var vertexShader = this._compileShader(this.ctx.VERTEX_SHADER, bohan.ajax.readText(vertexShaderUrl));
	try {
		var fragmentShader = this._compileShader(this.ctx.FRAGMENT_SHADER, bohan.ajax.readText(fragmentShaderUrl));
		try {
			var program = this.ctx.createProgram();
			if(program === 0) throw 'bohan.Gl.prototype.linkProgram: could not create program';
			this.ctx.attachShader(program, vertexShader);
			this.ctx.attachShader(program, fragmentShader);
			this.ctx.linkProgram(program);
			var linked = this.ctx.getProgramParameter(program, this.ctx.LINK_STATUS);
			if(!linked)	{
				var msg = this.ctx.GetProgramInfoLog(program);
				this.ctx.deleteProgram(program);
				throw('bohan.Gl.prototype.linkProgram: could not link program: ' + msg);
			}
			return program;
		} catch(e) {
			this.ctx.deleteShader(fragmentShader);
			throw e;
		}
	} catch(e) {
		this.ctx.deleteShader(vertexShader);
		throw e;
	}
}

