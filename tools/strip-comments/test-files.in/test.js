var a = 1. / 3 * 2;
// single line comment from start 'not a string ... still inside comment ... /* still the same comment
// single line comment from start "not a string ... still inside comment ... /* still the same comment
var b = 0; // single line comment at end of line
/* multi-line comment from start
	'not a string ... still inside comment
	"not a string ... still inside comment
	// still the same comment
	nested /* should be ignored
	no escape in comments \*/
var c /* multi-line
	comment in the middle of a statement */ = 0;
var d = 'inside string "still inside same string ... escaped \\ \' \" \0 \a \n \r \t still inside string /* not a comment // not a comment ... still inside string';
var e = "inside string 'still inside same string ... escaped \\ \" \" \0 \a \n \r \t still inside string /* not a comment // not a comment ... still inside string";

// blanks follows


		
	    

if(cond) { // indented comment
	// indented comment
	var f = 0;
	// indented comment, followed by blank lines
	

	if(cond2)
		/* idented comment */ var g = 0;
}
