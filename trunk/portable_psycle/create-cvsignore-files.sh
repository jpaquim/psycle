#!/bin/sh

function main
{
	(
		echo .cvsignore
		echo \*.suo
		echo \*.ncb
		echo output
	) > make\msvc_7.1\.cvsignore
	
	(
		echo .cvsignore
		echo boost
	) > include\.cvsignore
	
	(
		echo .cvsignore
		echo doxygen.mfc
	) > doc/.cvsignore
}

main "$@"
