var bohan = bohan || {};

// fairly complete: https://raw.github.com/getify/LABjs/master/LAB.src.js

bohan._included = {};

bohan.include = function(url) {
	if(bohan._included[url]) return;
	bohan._included[url] = true;
	var e = document.createElement('script');
	e.src = url;
	if(true) {
		// ie6 bug workaround
		document.head.insertBefore(e, document.head.firstChild);
	} else {
		document.head.appendChild(e);
	}
};

// window (global 'var window = this;' object: global variables of your script are in fact properties of window)
// window.self === window (the window itself)
// window.window === window (current window)
// window.opener (window)
// window.frames
// window.parent (window for frame)
// window.top (top most window in the hierarchy)
// window.showModalDialog()
// window.returnValue
// window.navigator
// window.screen
// window.document
// window.document.defaultView === window
// window.document.documentElement (root element of the document)
// window.onload (fired after document, images, subframes have finished loading)
// window.addEventListener('DOMContentLoaded', ...)

