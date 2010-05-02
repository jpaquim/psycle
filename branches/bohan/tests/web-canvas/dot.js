Graph = function() {
	this.vertices = [];
	this.edges = [];
	this.render = function(paper, x0, y0) {
		for(var i = 0; i < this.vertices.length; ++i) {
			var v = this.vertices[i];
			v.rank = 0;
		}
		for(var i = 0; i < this.edges.length; ++i) {
			var e = this.edges[i];
			var r = e.from.rank + 1;
			if(r > e.to.rank) e.to.rank = r;
		}
		var layers = [];
		for(var i = 0; i < this.vertices.length; ++i) {
			var v = this.vertices[i];
			var l = layers[v.rank];
			if(!l) l = layers[v.rank] = [];
			l.push(v);
		}
		for(var i = 0; i < layers.length; ++i) {
			var l = layers[i];
			var y = i * 50;
			for(var j = 0; j < l.length; ++j) {
				var v = l[j];
				var x = j * 50;
				v.render(paper, x0 + x, y0 + y);
			} 
		}
		for(var i = 0; i < this.edges.length; ++i) {
			var e = this.edges[i];
			e.render(paper);
		}
	}
}

Vertex = function(text) {
	this.text = text;
	this.render = function(paper, x, y) {
		this.x = x;
		this.y = y;
		var t = paper.text(x, y, this.text);
		var b = t.getBBox();
		var m = 5;
		var r = paper.rect(b.x - m, b.y - m, b.width + m * 2, b.height + m * 2, m);
		r.attr('fill', '#8dd');
		r.attr('stroke', '#000');
		r.insertBefore(t);
		r.rotate(5, x, y);
	}
}

Edge = function(from, to) {
	this.from = from;
	this.to = to;
	this.render = function(paper) {
		//paper.line(this.from.x, this.from.y, this.to.x, this.to.y);
	}
}

