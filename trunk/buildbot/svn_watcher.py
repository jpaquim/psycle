#! /usr/bin/env python

# This is a program which will poll a (remote) SVN repository, looking for
# new revisions. It then uses the 'buildbot sendchange' command to deliver
# information about the Change to a (remote) buildmaster. It can be run from
# a cron job on a periodic basis, or can be told (with the 'watch' option) to
# automatically repeat its check every 10 minutes.

# This script does not store any state information, so to avoid spurious
# changes you must use the 'watch' option and let it run forever.

# You will need to provide it with the location of the buildmaster's
# PBChangeSource port (in the form hostname:portnum), and the svnurl of the
# repository to watch.

# 2006.03.15 by John Pye
# 2006.03.29 by Niklaus Giger, added support to run under windows, added invocation option
# 2006.08.23 by Johan Boule, added support for multiple commits in the log

import commands
import xml.dom.minidom
import sys
import time
import os
if sys.platform == 'win32':
	import win32pipe

def checkChanges(repo, master, verbose=False, oldRevision = -1):
	if oldRevision >= 0:
		revisionRange = str(oldRevision + 1) + ':HEAD'
	else:
		revisionRange = 'HEAD'
		
	cmd = 'svn log --non-interactive --xml --verbose --revision ' + revisionRange + ' ' + repo

	if verbose == True:
		print "Getting last revision of repository: " + repo

	if sys.platform == 'win32':
		f = win32pipe.popen(cmd)
		xml1 = ''.join(f.readlines())
		f.close()
	else:
		xml1 = commands.getoutput(cmd)
    
	if verbose == True:
		print "XML\n-----------\n" + xml1 + "\n\n"
    
	doc = xml.dom.minidom.parseString(xml1)
	els = doc.getElementsByTagName("logentry")
	if els:
		for el in els:
			revision = int(el.getAttribute("revision"))
			author = "".join([t.data for t in
				el.getElementsByTagName("author")[0].childNodes])
			comments = "".join([t.data for t in
				el.getElementsByTagName("msg")[0].childNodes])
			
			pathlist = el.getElementsByTagName("paths")[0]
			paths = []
			for p in pathlist.getElementsByTagName("path"):
				paths.append("".join([t.data for t in p.childNodes]))
	
			if verbose == True:
				print "PATHS"
				print paths
	
			cmd = 'buildbot sendchange --master=' + master + ' --revision="' + str(revision) + '" --username="' + author + '" --comments="' + comments + '" ' + ' '.join(paths)
			
			if verbose == True:
				print cmd
			
			if sys.platform == 'win32':
				f = win32pipe.popen(cmd)
				print time.strftime("%H.%M.%S ") + "Revision " + revision + ": " + ''.join(f.readlines())
				f.close()
			else:
				xml1 = commands.getoutput(cmd)  
	else:
		print time.strftime("%H.%M.%S ") + "nothing has changed since revision " + str(oldRevision)
	return revision

if __name__ == '__main__':
	if len(sys.argv) == 4 and sys.argv[3] == 'watch':
		oldRevision = -1
		print "Watching for changes in repo "+  sys.argv[1] + " master " +  sys.argv[2] 
		while 1:
			oldRevision = checkChanges(sys.argv[1], sys.argv[2], False, oldRevision)
			time.sleep(5*60)

	elif len(sys.argv) == 3:
		checkChanges(sys.argv[1], sys.argv[2], True )
	else:
		print os.path.basename(sys.argv[0]) + ":  http://host/path/to/repo master:port [watch]"
