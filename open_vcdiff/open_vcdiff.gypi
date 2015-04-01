{
'targets': [
  {
    'target_name': 'open_vcdiff',
    'type': 'static_library',
    'dependencies': [
    ],
    'sources': [
      'src/addrcache.cc',
      'src/blockhash.cc',
      'src/blockhash.h',
      'src/checksum.h',
      'src/codetable.cc',
      'src/codetable.h',
      'src/compile_assert.h',
      'src/decodetable.cc',
      'src/decodetable.h',
      'src/encodetable.cc',
      'src/encodetable.h',
	  'src/google/format_extension_flags.h',
      'src/google/output_string.h',
      'src/google/vcdecoder.h',
      'src/google/vcencoder.h',
      'src/headerparser.cc',
      'src/headerparser.h',
      'src/instruction_map.cc',
      'src/instruction_map.h',
      'src/jsonwriter.h',
      'src/jsonwriter.cc',
      'src/logging.cc',
      'src/logging.h',
      'src/rolling_hash.h',
      'src/testing.h',
      'src/varint_bigendian.cc',
      'src/varint_bigendian.h',
      'src/vcdecoder.cc',
      'src/vcencoder.cc',
      'src/vcdiff_defs.h',
      'src/vcdiffengine.cc',
      'src/vcdiffengine.h',
      'src/zlib/adler32.c',
      'src/zlib/zconf.h',
      'src/zlib/zlib.h',
      'src/win/config.h',
	  'src/linux/config.h',
      'src/stdint.h',
    ],
    'include_dirs': [
	  'src',
	  'src/zlib',
    ],
	'conditions': [
      [ 'OS == "linux"', { 'include_dirs': [ 'linux' ] } ],
      [ 'OS == "win"', { 'include_dirs': [ 'win' ] } ],
    ],
	'defines': [
          'EXTERNAL_COMPILATION',
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
]
}
