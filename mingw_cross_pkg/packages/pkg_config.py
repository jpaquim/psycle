import os
from packages import Package

def package(packages): return PkgConfig(packages)

class PkgConfig(Package):
	def __init__(self, packages):
		Package.__init__(self, packages, 'pkg_config', '0.22')
		self.add_dep('gcc')

	def description(self): return 'manage compile and link flags for libraries'
	
	def _tarball(self): return self._dir() + '.tar.gz'
	def _dir(self): return 'pkg-config-' + self.version()

	def download(self): self.http_get('pkgconfig.freedesktop.org/releases/' + self._tarball())

	def clean_download(self): os.unlink(self._tarball())
	
	def build(self):
		self.shell('tar xzf ' + self._tarball())
		self.shell(
			'cd ' + self._dir() + ' && \n'
			'./configure \\\n'
				'--prefix=' + os.path.join(self.prefix(), self.target())
		)
		self.continue_build()

	def continue_build(self):
		self.shell(self.gmake() + ' -C ' + self._dir() + ' install DESTDIR=' + self.dest_dir())
		self.shell('install -d ' + os.path.join(self.prefix(), 'bin'))
		f = os.path.join(self.prefix(), 'bin', self.target() + '-pkg-config')
		if os.path.exists(f): os.unlink(f)
		self.shell('ln -s ' + os.path.join(os.pardir, self.target(), 'bin', 'pkg-config') + ' ' + f)
	
	def clean_build(self):
		self.shell('rm -Rf ' + self._dir())

