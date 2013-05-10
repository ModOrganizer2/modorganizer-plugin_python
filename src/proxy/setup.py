from distutils.core import setup, Extension
import sipdistutils
import os, sys

path = os.path.dirname(sys.argv[0])
if len(path) > 0:
	os.chdir(path)
os.environ['PATH'] += r";C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC"
print(os.environ['PATH'])

os.environ['VS90COMNTOOLS'] = os.environ['VS100COMNTOOLS']

setup(
	name = 'interfaces',
	versione = '1.0',
	ext_modules=[
		Extension("interfaces", [ "interfaces.sip" ],
			include_dirs=[ r"C:\QtSDK\Desktop\Qt\4.8.4\include",
										 r"C:\QtSDK\Desktop\Qt\4.8.4\include\QtCore",
										 path + r"\..\..\uibase" ],
			library_dirs=[ r"C:\QtSDK\Desktop\Qt\4.8.4\lib" ],
			libraries=[ "QtCore4" ]
		),
	],

	cmdclass = {'build_ext': sipdistutils.build_ext}
)
