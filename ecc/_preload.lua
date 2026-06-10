
	local p = premake

	newoption {
		trigger = "config",
		value = "CFG",
		description = "Select config for export compile_commands.json"
	}

	newaction {
		trigger         = "ecc",
		shortname       = "Export compile commands",
		description     = "Export compile_commands.json for language server",
		toolset         = "gcc",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		valid_languages = { "C", "C++" },
		valid_tools     = {
			cc     = { "clang", "gcc" }
		},

		onStart = function()
			p.indent("  ")
		end,

		execute = function()
			local dir = {}
			dir.location = _MAIN_SCRIPT_DIR
			p.generate(dir, "compile_commands.json", p.modules.ecc.generateFile)
		end
	}

	return function(cfg)
		return (_ACTION == "ecc")
	end
