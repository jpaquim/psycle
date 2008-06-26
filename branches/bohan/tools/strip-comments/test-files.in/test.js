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

var d /* multi-line
	comment followed by new line */
	= 0;

var e = 'inside string "still inside same string ... escaped \\ \' \" \0 \a \n \r \t still inside string /* not a comment // not a comment ... still inside string';
var f = "inside string 'still inside same string ... escaped \\ \" \" \0 \a \n \r \t still inside string /* not a comment // not a comment ... still inside string";

// blanks follows


		
	  var    s    =   '.  .    .     .'  ;

if(cond) { // indented comment
	// indented comment
	var g = 0;
	// indented comment, followed by blank lines
	

	if(cond2)
		/* idented comment */ var h = 0;
}
