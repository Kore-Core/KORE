from distutils.core import setup
setup(name='btcspendfrom',
      version='1.0',
      description='Command-line utility for kore "coin control"',
      author='Gavin Andresen',
      author_email='gavin@korefoundation.org',
      requires=['jsonrpc'],
      scripts=['spendfrom.py'],
      )
