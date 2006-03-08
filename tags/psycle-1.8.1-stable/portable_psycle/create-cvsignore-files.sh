#!/bin/sh

function main
{
	cd $(dirname $0) &&
	for i in 7.1 8.0
	do
		(
			echo .cvsignore
			echo \*.suo
			echo \*.ncb
			echo output
		) > make\msvc_$i\.cvsignore ||
		return
	done &&
	(
		echo .cvsignore
		echo boost
	) > include\.cvsignore &&
	(
		echo .cvsignore
		echo doxygen.mfc
	) > doc/.cvsignore
} &&

main "$@"
