--
-- Name:        premake-ninja/_preload.lua
-- Purpose:     Define the ninja action.
-- Author:      Dmitry Ivanov
-- Created:     2015/07/04
-- Copyright:   (c) 2015 Dmitry Ivanov
--

local p = premake

newaction {
	-- Metadata for the command line and help system
	trigger = 'ninja',
	shortname = 'ninja',
	description = 'Ninja is a small build system with a focus on speed',

	-- The capabilities of this action
	valid_kinds = { 'ConsoleApp', 'WindowedApp', 'SharedLib', 'StaticLib', 'None' }, -- Not supported: Makefile, Packaging, SharedItems, Utility
	valid_languages = { 'C', 'C++' },
	valid_tools = { cc = { 'gcc', 'clang', 'msc', 'emcc' } },

	toolset = iif(
		os.target() == 'windows',
		'msc-v142', -- Visual Studio 2019
		iif(os.target() == 'macosx', 'clang', 'gcc')
	),

	-- Workspace and project generation logic
	onWorkspace = function(wks)
		p.eol('\r\n')
		p.indent('  ')
		p.generate(wks, 'build.ninja', p.modules.ninja.generateWorkspace)
	end,
	onProject = function(prj)
		p.eol('\r\n')
		p.indent('  ')
		p.modules.ninja.generateProject(prj)
	end,
	onBranch = function(prj)
		p.eol('\r\n')
		p.indent('  ')
		p.modules.ninja.generateProject(prj)
	end,
	onCleanSolution = function(sln)
		-- TODO
	end,
	onCleanProject = function(prj)
		-- TODO
	end,
	onCleanTarget = function(prj)
		-- TODO
	end,
}

--
-- Decide when the full module should be loaded.
--

return function(cfg)
	return (_ACTION == 'ninja')
end
