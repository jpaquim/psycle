#! /usr/bin/env python

if __name__ == '__main__':
	import sys, os
	dir = os.path.abspath(os.path.dirname(__file__))
	if dir not in sys.path: sys.path.append(dir)
	from wonderbuild.main import main
	main()
