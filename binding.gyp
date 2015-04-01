{
  "targets": [
    {
      "target_name": "simple_vcdiff",
	  'dependencies': [
        'open_vcdiff',
      ],
      "sources": [ "main.cc" ],
	  'include_dirs': [
		'open_vcdiff/src'
      ],
	  'cflags!': [ '-fno-exceptions' ],
	  'cflags_cc!': [ '-fno-exceptions' ],
	  'configurations': {
	    'Release': {
	      'msvs_settings': {
		    'VCCLCompilerTool': {
		      'ExceptionHandling': 1
		    }
		  }
	    }
	  },
    }
  ],
  
  'includes': [ 'open_vcdiff/open_vcdiff.gypi' ],
}