{
	'targets': [
		{
			'target_name': 'zlid3',
			'link_settings': {
				'library_dirs' : ['../lib/macosx'],
				'libraries': ['libid3tag.a', 'libz.a']
      		},
			'sources': [ 'src/binding.cpp', 'src/fastheap.cpp', 'src/fastdict.cpp', 'src/array.cpp', 'src/hash.cpp', 'src/macosx.cpp', 'src/id3lib.cpp' ],
			'conditions': [
				['OS=="mac"', {
					'xcode_settings': {
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          			}
        		}]
        	]
		}
	]
}
