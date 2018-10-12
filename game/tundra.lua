Build {
	Units = "units.lua",
	Configs = {
		{
			Name = "win32-msvc",
			DefaultOnHost = "windows",
			Tools = { "msvc-vs2012" },
			Env = {
				CXXOPTS = { "/EHsc" },
			},
		},
	},
}
