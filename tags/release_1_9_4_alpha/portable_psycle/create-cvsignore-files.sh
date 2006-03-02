#! /bin/bash
# cygwin's sh not posix
# !/bin/sh

function main
{
	dirname $0 && # first test there's no space in the path
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
