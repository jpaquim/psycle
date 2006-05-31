class source_package:
	def __init__(
		self,
		packageneric,
		name = None,
		version = None,
		description = '',
		long_description = '',
		path = ''
	):
		self._packageneric = packageneric
		self._name = name
		self._version = version
		self._description= description
		self._long_description = long_description
		self._path = path
		self.packageneric().environment().Append(
			CPPDEFINES = {
				'PACKAGENERIC': '\\"/dev/null\\"',
				'PACKAGENERIC__PACKAGE__NAME': '\\"' + self.name() + '\\"',
				'PACKAGENERIC__PACKAGE__VERSION': '\\"' + str(self.version()) + '\\"',
				'PACKAGENERIC__PACKAGE__VERSION__MAJOR': str(self.version().major()),
				'PACKAGENERIC__PACKAGE__VERSION__MINOR': str(self.version().minor()),
				'PACKAGENERIC__PACKAGE__VERSION__PATCH': str(self.version().patch())
			}
		)
	
	def packageneric(self):
		return self._packageneric
	
	def name(self):
		return self._name
		
	def version(self):
		return self._version
		
	def description(self):
		return self._description
		
	def long_description(self):
		return self._long_description
		
	def path(self):
		return self._path
