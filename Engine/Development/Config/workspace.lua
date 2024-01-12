workspace('Snowbite')
	location(root_directory)

	configurations {
		'Debug',
		'Release',
		'Distribution',
	}

	platforms {
		'Windows',
	}

	defaultplatform 'Windows'

	filter 'platforms:Windows'
		architecture 'x86_64'
		system 'Windows'
		systemversion 'latest'

  filter { }

  defines {
    'SB_BUILD_MONOLITHIC=1'
  }