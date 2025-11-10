--
-- Name:        premake-ninja/ninja.lua
-- Purpose:     Define the ninja action.
-- Author:      Dmitry Ivanov
-- Created:     2015/07/04
-- Copyright:   (c) 2015 Dmitry Ivanov
--

local p = premake
local tree = p.tree
local project = p.project
local config = p.config
local fileconfig = p.fileconfig

-- Some toolset fixes/helper
p.tools.clang.objectextension = '.o'
p.tools.gcc.objectextension = '.o'
p.tools.msc.objectextension = '.obj'

p.tools.clang.tools.rc = p.tools.clang.tools.rc or 'windres'

p.tools.msc.gettoolname = function(cfg, name)
	local map = { cc = 'cl', cxx = 'cl', ar = 'lib', rc = 'rc' }
	return map[name]
end

-- Ninja module
premake.modules.ninja = {}
local ninja = p.modules.ninja

ninja.handlers = {}

function ninja.register_handler(kind, compilation_rules, target_rules)
	ninja.handlers[kind] = { compilation_rules = compilation_rules, target_rules = target_rules }
end

local function get_key(cfg, name)
	local name = name or cfg.project.name

	if cfg.platform then
		return name .. '_' .. cfg.buildcfg .. '_' .. cfg.platform
	else
		return name .. '_' .. cfg.buildcfg
	end
end

local build_cache = {}

function ninja.add_build(cfg, out, implicit_outputs, command, inputs, implicit_inputs, dependencies, vars)
	implicit_outputs = ninja.list(table.translate(implicit_outputs, ninja.esc))
	if #implicit_outputs > 0 then
		implicit_outputs = ' |' .. implicit_outputs
	else
		implicit_outputs = ''
	end

	inputs = ninja.list(table.translate(inputs, ninja.esc))

	implicit_inputs = ninja.list(table.translate(implicit_inputs, ninja.esc))
	if #implicit_inputs > 0 then
		implicit_inputs = ' |' .. implicit_inputs
	else
		implicit_inputs = ''
	end

	dependencies = ninja.list(table.translate(dependencies, ninja.esc))
	if #dependencies > 0 then
		dependencies = ' ||' .. dependencies
	else
		dependencies = ''
	end
	build_line = 'build ' .. ninja.esc(out) .. implicit_outputs .. ': ' .. command .. inputs .. implicit_inputs .. dependencies

	local cached = build_cache[out]
	if cached ~= nil then
		if build_line == cached.build_line and table.equals(vars or {}, cached.vars or {}) then
			-- custom_command/copy rule are identical for each configuration (contrary to other rules)
			-- So we can compare extra parameter
			if command == 'custom_command' or command == 'copy' then
				p.outln('# INFO: Rule ignored, same as ' .. cached.cfg_key)
			else
				local cfg_key = cfg and get_key(cfg) or 'Global scope'
				p.warn(cached.cfg_key .. ' and ' .. cfg_key .. ' both generate (differently?) ' .. out .. '. Ignoring ' .. cfg_key)
				p.outln('# WARNING: Rule ignored, using the one from ' .. cached.cfg_key)
			end
		else
			local cfg_key = cfg and get_key(cfg) or 'Global scope'
			p.warn(cached.cfg_key .. ' and ' .. cfg_key .. ' both generate differently ' .. out .. '. Ignoring ' .. cfg_key)
			p.outln('# ERROR: Rule ignored, using the one from ' .. cached.cfg_key)
		end
		p.outln('# ' .. build_line)
		for i, var in ipairs(vars or {}) do
			p.outln('#   ' .. var)
		end
		return
	end
	p.outln(build_line)
	for i, var in ipairs(vars or {}) do
		p.outln('  ' .. var)
	end
	build_cache[out] = {
		cfg_key = cfg and get_key(cfg) or 'Global scope',
		build_line = build_line,
		vars = vars,
	}
end

function ninja.emit_rule(name, cmds, description, opts)
	opts = opts or {}
	p.outln('rule ' .. name)
	p.outln('  command = ' .. table.concat(cmds, ' &&$\n            '))
	p.outln('  description = ' .. description)
	for key, value in pairs(opts) do
		p.outln('  ' .. key .. ' = ' .. value)
	end
	p.outln('')
end

function ninja.emit_flags(name, value)
	p.outln(name .. '=' .. value)
end

function ninja.esc(value)
	value = value:gsub('%$', '$$') -- TODO maybe there is better way
	value = value:gsub(':', '$:')
	value = value:gsub('\n', '$\n')
	value = value:gsub(' ', '$ ')
	return value
end

function ninja.quote(value)
	value = value:gsub('\\', '\\\\')
	value = value:gsub("'", "\\'")
	value = value:gsub('"', '\\"')

	return '"' .. value .. '"'
end

-- in some cases we write file names in rule commands directly
-- so we need to propely escape them
function ninja.shesc(value)
	if type(value) == 'table' then
		local result = {}
		local n = #value
		for i = 1, n do
			table.insert(result, ninja.shesc(value[i]))
		end
		return result
	end

	if value:find(' ') or value:find('"') or value:find('(', 1, true) or value:find(')') or value:find('|') or value:find('&') then
		return ninja.quote(value)
	end
	return value
end

function ninja.can_generate(prj)
	return p.action.supports(prj.kind) and prj.kind ~= p.NONE
end

-- generate solution that will call ninja for projects
function ninja.generateWorkspace(wks)
	local oldGetDefaultSeparator = path.getDefaultSeparator
	path.getDefaultSeparator = function()
		return '/'
	end

	p.outln('# solution build file')
	p.outln('# generated with premake ninja')
	p.outln('')

	p.outln('# build projects')
	local cfgs = {} -- key is concatenated name or variant name, value is string of outputs names
	local key = ''
	local cfg_first = nil
	local cfg_first_lib = nil
	local subninjas = {}

	for prj in p.workspace.eachproject(wks) do
		if ninja.can_generate(prj) then
			for cfg in p.project.eachconfig(prj) do
				key = get_key(cfg)

				if not cfgs[cfg.buildcfg] then
					cfgs[cfg.buildcfg] = {}
				end
				table.insert(cfgs[cfg.buildcfg], key)

				-- set first configuration name
				if wks.defaultplatform == nil then
					if (cfg_first == nil) and (cfg.kind == p.CONSOLEAPP or cfg.kind == p.WINDOWEDAPP) then
						cfg_first = key
					end
				end
				if (cfg_first_lib == nil) and (cfg.kind == p.STATICLIB or cfg.kind == p.SHAREDLIB) then
					cfg_first_lib = key
				end
				if prj.name == wks.startproject then
					if wks.defaultplatform == nil then
						cfg_first = key
					elseif cfg.platform == wks.defaultplatform then
						if cfg_first == nil then
							cfg_first = key
						end
					end
				end

				-- include other ninja file
				table.insert(subninjas, ninja.esc(ninja.projectCfgFilename(cfg, true)))
				p.outln('subninja ' .. ninja.esc(ninja.projectCfgFilename(cfg, true)))
			end
		end
	end

	if cfg_first == nil then
		cfg_first = cfg_first_lib
	end

	p.outln('')

	p.outln('# targets')
	for cfg, outputs in spairs(cfgs) do
		p.outln('build ' .. ninja.esc(cfg) .. ': phony' .. ninja.list(table.translate(outputs, ninja.esc)))
	end
	p.outln('')

	if wks.editorintegration then
		-- we need to filter out the 'file' argument, since we already output
		-- the script separately.
		local args = {}
		for _, arg in ipairs(_ARGV) do
			if not (arg:startswith('--file') or arg:startswith('/file')) then
				table.insert(args, arg)
			end
		end
		table.sort(args)

		p.outln('# Rule')
		ninja.emit_rule('premake', { ninja.shesc(p.workspace.getrelative(wks, _PREMAKE_COMMAND)) .. ' --file=$in ' .. table.concat(ninja.shesc(args), ' ') }, 'run premake', { generator = 'true', restat = 'true' })
		ninja.add_build(nil, 'build.ninja', subninjas, 'premake', { p.workspace.getrelative(wks, _MAIN_SCRIPT) }, {}, {}, {})
		p.outln('')
	end

	if cfg_first then
		p.outln('# default target')
		p.outln('default ' .. ninja.esc(cfg_first))
		p.outln('')
	end

	path.getDefaultSeparator = oldGetDefaultSeparator
end

function ninja.list(value)
	if #value > 0 then
		return ' ' .. table.concat(value, ' ')
	else
		return ''
	end
end

local function shouldcompileasc(filecfg)
	if filecfg.compileas and filecfg.compileas ~= 'Default' then
		return p.languages.isc(filecfg.compileas)
	end
	return path.iscfile(filecfg.abspath)
end

local function shouldcompileascpp(filecfg)
	if filecfg.compileas and filecfg.compileas ~= 'Default' then
		return p.languages.iscpp(filecfg.compileas)
	end
	return path.iscppfile(filecfg.abspath)
end

local function getFileDependencies(cfg)
	local dependencies = {}
	if #cfg.prebuildcommands > 0 or cfg.prebuildmessage then
		dependencies = { 'prebuild_' .. get_key(cfg) }
	end
	for i = 1, #cfg.dependson do
		local dependpostfix = ''
		if cfg.platform then
			dependpostfix = '_' .. cfg.platform
		end

		table.insert(dependencies, cfg.dependson[i] .. '_' .. cfg.buildcfg .. dependpostfix)
	end
	return dependencies
end

local function getcflags(toolset, cfg, filecfg)
	p.escaper(ninja.shesc)
	local buildopt = ninja.list(filecfg.buildoptions)
	local cppflags = ninja.list(toolset.getcppflags(filecfg))
	local cflags = ninja.list(toolset.getcflags(filecfg))
	local defines = ninja.list(table.join(toolset.getdefines(filecfg.defines, filecfg), toolset.getundefines(filecfg.undefines)))
	local includes = ninja.list(toolset.getincludedirs(cfg, filecfg.includedirs, filecfg.externalincludedirs, filecfg.frameworkdirs, filecfg.includedirsafter))
	local forceincludes = ninja.list(toolset.getforceincludes(cfg))
	p.escaper(nil)

	return buildopt .. cppflags .. cflags .. defines .. includes .. forceincludes
end

local function getcxxflags(toolset, cfg, filecfg)
	p.escaper(ninja.shesc)
	local buildopt = ninja.list(filecfg.buildoptions)
	local cppflags = ninja.list(toolset.getcppflags(filecfg))
	local cxxflags = ninja.list(toolset.getcxxflags(filecfg))
	local defines = ninja.list(table.join(toolset.getdefines(filecfg.defines, filecfg), toolset.getundefines(filecfg.undefines)))
	local includes = ninja.list(toolset.getincludedirs(cfg, filecfg.includedirs, filecfg.externalincludedirs, filecfg.frameworkdirs, filecfg.includedirsafter))
	local forceincludes = ninja.list(toolset.getforceincludes(cfg))
	p.escaper(nil)

	return buildopt .. cppflags .. cxxflags .. defines .. includes .. forceincludes
end

local function getldflags(toolset, cfg)
	local ldflags = ninja.list(table.join(toolset.getLibraryDirectories(cfg), toolset.getrunpathdirs(cfg, table.join(cfg.runpathdirs, config.getsiblingtargetdirs(cfg))), toolset.getldflags(cfg), cfg.linkoptions))

	-- experimental feature, change install_name of shared libs
	--if (toolset == p.tools.clang) and (cfg.kind == p.SHAREDLIB) and ninja.endsWith(cfg.buildtarget.name, ".dylib") then
	--	ldflags = ldflags .. " -install_name " .. cfg.buildtarget.name
	--end

	return ldflags
end

local function getresflags(toolset, cfg, filecfg)
	p.escaper(ninja.shesc)
	local defines = ninja.list(toolset.getdefines(table.join(filecfg.defines, filecfg.resdefines), filecfg))
	local includes = ninja.list(toolset.getincludedirs(cfg, table.join(filecfg.externalincludedirs, filecfg.includedirsafter, filecfg.includedirs, filecfg.resincludedirs), {}, {}, {}))
	local options = ninja.list(cfg.resoptions)
	p.escaper(nil)

	return defines .. includes .. options
end

local function prebuild_rule(cfg)
	if #cfg.prebuildcommands > 0 or cfg.prebuildmessage then
		local commands = {}
		if cfg.prebuildmessage then
			commands = { os.translateCommandsAndPaths('{ECHO} ' .. cfg.prebuildmessage, cfg.workspace.basedir, cfg.workspace.location) }
		end
		commands = table.join(commands, os.translateCommandsAndPaths(cfg.prebuildcommands, cfg.workspace.basedir, cfg.workspace.location))
		if #commands > 1 then
			commands = 'sh -c ' .. ninja.quote(table.implode(commands, '', '', ';'))
		else
			commands = commands[1]
		end
		ninja.emit_rule('run_prebuild', { commands }, 'prebuild')
	end
end

local function prelink_rule(cfg)
	if #cfg.prelinkcommands > 0 or cfg.prelinkmessage then
		local commands = {}
		if cfg.prelinkmessage then
			commands = { os.translateCommandsAndPaths('{ECHO} ' .. cfg.prelinkmessage, cfg.workspace.basedir, cfg.workspace.location) }
		end
		commands = table.join(commands, os.translateCommandsAndPaths(cfg.prelinkcommands, cfg.workspace.basedir, cfg.workspace.location))
		if #commands > 1 then
			commands = 'sh -c ' .. ninja.quote(table.implode(commands, '', '', ';'))
		else
			commands = commands[1]
		end
		ninja.emit_rule('run_prelink', { commands }, 'prelink')
	end
end

local function postbuild_rule(cfg)
	if #cfg.postbuildcommands > 0 or cfg.postbuildmessage then
		local commands = {}
		if cfg.postbuildmessage then
			commands = { os.translateCommandsAndPaths('{ECHO} ' .. cfg.postbuildmessage, cfg.workspace.basedir, cfg.workspace.location) }
		end
		commands = table.join(commands, os.translateCommandsAndPaths(cfg.postbuildcommands, cfg.workspace.basedir, cfg.workspace.location))
		if #commands > 1 then
			commands = 'sh -c ' .. ninja.quote(table.implode(commands, '', '', ';'))
		else
			commands = commands[1]
		end
		ninja.emit_rule('run_postbuild', { commands }, 'postbuild')
	end
end

local function c_cpp_compilation_rules(cfg, toolset, pch)
	---------------------------------------------------- figure out toolset executables
	local cc = toolset.gettoolname(cfg, 'cc')
	local cxx = toolset.gettoolname(cfg, 'cxx')
	local ar = toolset.gettoolname(cfg, 'ar')
	local link = toolset.gettoolname(cfg, iif(cfg.language == 'C', 'cc', 'cxx'))
	local rc = toolset.gettoolname(cfg, 'rc')

	-- all paths need to be relative to the workspace output location,
	-- and not relative to the project output location.
	-- override the toolset getrelative function to achieve this

	local getrelative = p.tools.getrelative
	p.tools.getrelative = function(cfg, value)
		return p.workspace.getrelative(cfg.workspace, value)
	end

	local all_cflags = getcflags(toolset, cfg, cfg)
	local all_cxxflags = getcxxflags(toolset, cfg, cfg)
	local all_ldflags = getldflags(toolset, cfg)
	local all_resflags = getresflags(toolset, cfg, cfg)

	if toolset == p.tools.msc then
		ninja.emit_flags('CFLAGS', all_cflags)
		ninja.emit_rule('cc', { cc .. ' $CFLAGS /nologo /showIncludes -c /Tc$in /Fo$out' }, 'cc $out', { deps = 'msvc' })

		ninja.emit_flags('CXXFLAGS', all_cxxflags)
		ninja.emit_rule('cxx', { cxx .. ' $CXXFLAGS /nologo /showIncludes -c /Tp$in /Fo$out' }, 'cxx $out', { deps = 'msvc' })

		ninja.emit_flags('CFLAGS', all_cflags)
		ninja.emit_rule('clangtidy_cc', { 'clang-tidy $in -- -x c $CFLAGS', cc .. ' $CFLAGS /nologo /showIncludes -c /Tc$in /Fo$out' }, 'cc $out', { deps = 'msvc' })

		ninja.emit_flags('CXXFLAGS', all_cxxflags)
		ninja.emit_rule('clangtidy_cxx', { 'clang-tidy $in -- -x c++ $CFLAGS', cxx .. ' $CXXFLAGS /nologo /showIncludes -c /Tp$in /Fo$out' }, 'cxx $out', { deps = 'msvc' })

		ninja.emit_flags('RESFLAGS', all_resflags)
		ninja.emit_rule('rc', { rc .. ' /nologo /fo$out $in $RESFLAGS' }, 'rc $out')

		if cfg.kind == p.STATICLIB then
			ninja.emit_rule('ar', { ar .. ' $in /nologo -OUT:$out' }, 'ar $out')
		else
			ninja.emit_rule('link', { link .. ' $in ' .. ninja.list(ninja.shesc(toolset.getlinks(cfg, true))) .. ' /link ' .. all_ldflags .. ' /nologo /out:$out' }, 'link $out')
		end
	elseif toolset == p.tools.clang or toolset == p.tools.gcc or toolset == p.tools.emcc then
		local force_include = pch and (' -include ' .. ninja.shesc(pch.placeholder)) or ''

		if pch then
			ninja.emit_rule('build_pch', { iif(cfg.language == 'C', cc .. all_cflags .. ' -x c-header', cxx .. all_cxxflags .. ' -x c++-header') .. ' -H -MF $out.d -c -o $out $in' }, 'build_pch $out', { depfile = '$out.d', deps = 'gcc' })
		end

		ninja.emit_flags('CFLAGS', all_cflags)
		ninja.emit_rule('cc', { cc .. ' $CFLAGS' .. force_include .. ' -x c -MF $out.d -c -o $out $in' }, 'cc $out', { depfile = '$out.d', deps = 'gcc' })

		ninja.emit_flags('CXXFLAGS', all_cxxflags)
		ninja.emit_rule('cxx', { cxx .. ' $CXXFLAGS' .. force_include .. ' -x c++ -MF $out.d -c -o $out $in' }, 'cxx $out', { depfile = '$out.d', deps = 'gcc' })

		ninja.emit_flags('CFLAGS', all_cflags)
		ninja.emit_rule('clangtidy_cc', { 'clang-tidy $in -- -x c $CFLAGS' .. force_include, cc .. ' $CFLAGS' .. force_include .. ' -x c -MF $out.d -c -o $out $in' }, 'cc $out', { depfile = '$out.d', deps = 'gcc' })

		ninja.emit_flags('CXXFLAGS', all_cxxflags)
		ninja.emit_rule('clangtidy_cxx', { 'clang-tidy $in -- -x c++ $CFLAGS' .. force_include, cxx .. ' $CXXFLAGS' .. force_include .. '-x c++ -MF $out.d -c -o $out $in' }, 'cxx $out', { depfile = '$out.d', deps = 'gcc' })

		ninja.emit_flags('RESFLAGS', all_resflags)
		if rc then
			ninja.emit_rule('rc', { rc .. ' -i $in -o $out $RESFLAGS' }, 'rc $out')
		end

		if ar and cfg.kind == p.STATICLIB then
			ninja.emit_rule('ar', { ar .. ' rcs $out $in' }, 'ar $out')
		else
			local groups = iif(cfg.linkgroups == premake.ON, { '-Wl,--start-group ', ' -Wl,--end-group' }, { '', '' })
			ninja.emit_rule('link', { link .. ' -o $out ' .. groups[1] .. '$in' .. ninja.list(ninja.shesc(toolset.getlinks(cfg, true, true))) .. all_ldflags .. groups[2] }, 'link $out')
		end
	end

	p.tools.getrelative = getrelative
end

local function custom_command_rule()
	ninja.emit_rule('custom_command', { '$CUSTOM_COMMAND' }, '$CUSTOM_DESCRIPTION')
end

local function copy_rule()
	ninja.emit_rule('copy', { os.translateCommands('{COPYFILE} $in $out') }, 'copy $in $out')
end

local function collect_generated_files(prj, cfg)
	local generated_files = {}
	tree.traverse(project.getsourcetree(prj), {
		onleaf = function(node, depth)
			function append_to_generated_files(filecfg)
				local outputs = project.getrelative(prj.workspace, filecfg.buildoutputs)
				generated_files = table.join(generated_files, outputs)
			end
			local filecfg = fileconfig.getconfig(node, cfg)
			if not filecfg or filecfg.flags.ExcludeFromBuild then
				return
			end
			local rule = p.global.getRuleForFile(node.name, prj.rules)
			if fileconfig.hasCustomBuildRule(filecfg) then
				append_to_generated_files(filecfg)
			elseif rule then
				local environ = table.shallowcopy(filecfg.environ)

				if rule.propertydefinition then
					p.rule.prepareEnvironment(rule, environ, cfg)
					p.rule.prepareEnvironment(rule, environ, filecfg)
				end
				local rulecfg = p.context.extent(rule, environ)
				append_to_generated_files(rulecfg)
			end
		end,
	}, false, 1)
	return generated_files
end

local function pch_build(cfg, pch)
	local pch_dependency = {}
	if pch then
		pch_dependency = { pch.gch }
		ninja.add_build(cfg, pch.gch, {}, 'build_pch', { pch.input }, {}, {}, {})
	end
	return pch_dependency
end

local function custom_command_build(prj, cfg, filecfg, filename, file_dependencies)
	local outputs = project.getrelative(prj.workspace, filecfg.buildoutputs)
	local output = outputs[1]
	table.remove(outputs, 1)
	local commands = {}
	if filecfg.buildmessage then
		commands = { os.translateCommandsAndPaths('{ECHO} ' .. filecfg.buildmessage, prj.workspace.basedir, prj.workspace.location) }
	end
	commands = table.join(commands, os.translateCommandsAndPaths(filecfg.buildcommands, prj.workspace.basedir, prj.workspace.location))
	if #commands > 1 then
		commands = 'sh -c ' .. ninja.quote(table.implode(commands, '', '', ';'))
	else
		commands = commands[1]
	end

	ninja.add_build(cfg, output, outputs, 'custom_command', { filename }, project.getrelative(prj.workspace, filecfg.buildinputs), file_dependencies, { 'CUSTOM_COMMAND = ' .. commands, 'CUSTOM_DESCRIPTION = custom build ' .. ninja.shesc(output) })
end

local function compile_file_build(cfg, filecfg, toolset, pch_dependency, regular_file_dependencies, objfiles, extrafiles)
	local obj_file = filecfg.objname .. (toolset.objectextension or '.o')
	local obj_dir = project.getrelative(cfg.workspace, cfg.objdir)
	local filepath = project.getrelative(cfg.workspace, filecfg.abspath)
	local has_custom_settings = fileconfig.hasFileSettings(filecfg)
	local use_clangtidy = filecfg.clangtidy or (filecfg.clangtidy == nil and cfg.clangtidy)

	if filecfg.buildaction == 'None' then
		return
	elseif filecfg.buildaction == 'Copy' then
		local target = project.getrelative(cfg.workspace, path.join(cfg.targetdir, filecfg.name))
		ninja.add_build(cfg, target, {}, 'copy', { filepath }, {}, {}, {})
		extrafiles[#extrafiles + 1] = target
	elseif shouldcompileasc(filecfg) then
		local objfilename = obj_dir .. '/' .. obj_file
		objfiles[#objfiles + 1] = objfilename
		local vars = {}
		if has_custom_settings then
			cflags = 'CFLAGS = $CFLAGS ' .. getcflags(toolset, cfg, filecfg)
			vars = { cflags }
		end
		ninja.add_build(cfg, objfilename, {}, iif(use_clangtidy, 'clangtidy_cc', 'cc'), { filepath }, pch_dependency, regular_file_dependencies, vars)
	elseif shouldcompileascpp(filecfg) then
		local objfilename = obj_dir .. '/' .. obj_file
		objfiles[#objfiles + 1] = objfilename
		local vars = {}
		if has_custom_settings then
			cxxflags = 'CXXFLAGS = $CXXFLAGS ' .. getcxxflags(toolset, cfg, filecfg)
			vars = { cxxflags }
		end
		ninja.add_build(cfg, objfilename, {}, iif(use_clangtidy, 'clangtidy_cxx', 'cxx'), { filepath }, pch_dependency, regular_file_dependencies, vars)
	elseif path.isresourcefile(filecfg.abspath) then
		local objfilename = obj_dir .. '/' .. filecfg.basename .. '.res'
		objfiles[#objfiles + 1] = objfilename
		local resflags = {}
		if has_custom_settings then
			resflags = { 'RESFLAGS = $RESFLAGS ' .. getresflags(toolset, cfg, filecfg) }
		end
		local rc = toolset.gettoolname(cfg, 'rc')
		if rc then
			ninja.add_build(cfg, objfilename, {}, 'rc', { filepath }, {}, {}, resflags)
		else
			p.warnOnce(filepath, string.format('Ignored resource: "%s"', filepath))
		end
	end
end

local function files_build(prj, cfg, toolset, pch_dependency, regular_file_dependencies, file_dependencies)
	local objfiles = {}
	local extrafiles = {}
	tree.traverse(project.getsourcetree(prj), {
		onleaf = function(node, depth)
			local filecfg = fileconfig.getconfig(node, cfg)
			if not filecfg or filecfg.flags.ExcludeFromBuild then
				return
			end
			local rule = p.global.getRuleForFile(node.name, prj.rules)
			local filepath = project.getrelative(cfg.workspace, node.abspath)

			if fileconfig.hasCustomBuildRule(filecfg) then
				custom_command_build(prj, cfg, filecfg, filepath, file_dependencies)
			elseif rule then
				local environ = table.shallowcopy(filecfg.environ)

				if rule.propertydefinition then
					p.rule.prepareEnvironment(rule, environ, cfg)
					p.rule.prepareEnvironment(rule, environ, filecfg)
				end
				local rulecfg = p.context.extent(rule, environ)
				custom_command_build(prj, cfg, rulecfg, filepath, file_dependencies)
			else
				compile_file_build(cfg, filecfg, toolset, pch_dependency, regular_file_dependencies, objfiles, extrafiles)
			end
		end,
	}, false, 1)
	p.outln('')

	return objfiles, extrafiles
end

local function generated_files_build(cfg, generated_files, key)
	local final_dependency = {}
	if #generated_files > 0 then
		p.outln('# generated files')
		ninja.add_build(cfg, 'generated_files_' .. key, {}, 'phony', generated_files, {}, {}, {})
		final_dependency = { 'generated_files_' .. key }
	end
	return final_dependency
end

-- generate project + config build file
function ninja.generateProjectCfg(cfg)
	local oldGetDefaultSeparator = path.getDefaultSeparator
	path.getDefaultSeparator = function()
		return '/'
	end

	local prj = cfg.project
	local key = get_key(cfg)
	local toolset, toolset_version = p.tools.canonical(cfg.toolset)

	if not toolset then
		p.error('Unknown toolset ' .. cfg.toolset)
	end

	-- Some toolset fixes
	cfg.gccprefix = cfg.gccprefix or ''

	p.outln('# project build file')
	p.outln('# generated with premake ninja')
	p.outln('')

	-- premake-ninja relies on scoped rules
	-- and they were added in ninja v1.6
	p.outln('ninja_required_version = 1.6')
	p.outln('')

	local is_c_or_cpp = cfg.language == p.C or cfg.language == p.CPP

	---------------------------------------------------- figure out settings
	local pch = nil
	if is_c_or_cpp then
		if toolset ~= p.tools.msc then
			pch = p.tools.gcc.getpch(cfg)
			if pch then
				pch = {
					input = pch,
					placeholder = project.getrelative(cfg.workspace, path.join(cfg.objdir, path.getname(pch))),
					gch = project.getrelative(cfg.workspace, path.join(cfg.objdir, path.getname(pch) .. '.gch')),
				}
			end
		end
	end

	---------------------------------------------------- write rules
	p.outln('# core rules for ' .. cfg.name)
	prebuild_rule(cfg)
	prelink_rule(cfg)
	postbuild_rule(cfg)

	if is_c_or_cpp then
		c_cpp_compilation_rules(cfg, toolset, pch)
	else
		local handler = ninja.handlers[cfg.language]
		if not handler then
			p.error('expected registered ninja handler action for target ' .. cfg.language)
		end
		handler.compilation_rules(cfg, toolset)
	end

	copy_rule()
	custom_command_rule()

	---------------------------------------------------- build all files
	p.outln('# build files')

	local pch_dependency = is_c_or_cpp and pch_build(cfg, pch) or {}

	local generated_files = collect_generated_files(prj, cfg)

	local file_dependencies = getFileDependencies(cfg)
	local regular_file_dependencies = table.join(iif(#generated_files > 0, { 'generated_files_' .. key }, {}), file_dependencies)

	local obj_dir = project.getrelative(cfg.workspace, cfg.objdir)
	local objfiles, extrafiles = files_build(prj, cfg, toolset, pch_dependency, regular_file_dependencies, file_dependencies)
	local final_dependency = generated_files_build(cfg, generated_files, key)

	---------------------------------------------------- build final target
	if #cfg.prebuildcommands > 0 or cfg.prebuildmessage then
		p.outln('# prebuild')
		ninja.add_build(cfg, 'prebuild_' .. get_key(cfg), {}, 'run_prebuild', {}, {}, {}, {})
	end
	local prelink_dependency = {}
	if #cfg.prelinkcommands > 0 or cfg.prelinkmessage then
		p.outln('# prelink')
		ninja.add_build(cfg, 'prelink_' .. get_key(cfg), {}, 'run_prelink', {}, objfiles, final_dependency, {})
		prelink_dependency = { 'prelink_' .. get_key(cfg) }
	end
	if #cfg.postbuildcommands > 0 or cfg.postbuildmessage then
		p.outln('# postbuild')
		ninja.add_build(cfg, 'postbuild_' .. get_key(cfg), {}, 'run_postbuild', {}, { ninja.outputFilename(cfg) }, {}, {})
	end

	if is_c_or_cpp then
		-- we don't pass getlinks(cfg) through dependencies
		-- because system libraries are often not in PATH so ninja can't find them
		local libs = table.translate(config.getlinks(cfg, 'siblings', 'fullpath'), function(p)
			return project.getrelative(cfg.workspace, path.join(cfg.project.location, p))
		end)
		local cfg_output = ninja.outputFilename(cfg)
		local extra_outputs = {}
		local command_rule = ''
		if cfg.kind == p.STATICLIB then
			p.outln('# link static lib')
			command_rule = 'ar'
		elseif cfg.kind == p.SHAREDLIB then
			p.outln('# link shared lib')
			command_rule = 'link'
			extra_outputs = iif(os.target() == 'windows', { path.replaceextension(cfg_output, '.lib'), path.replaceextension(cfg_output, '.exp') }, {})
		elseif (cfg.kind == p.CONSOLEAPP) or (cfg.kind == p.WINDOWEDAPP) then
			p.outln('# link executable')
			command_rule = 'link'
		else
			p.error("ninja action doesn't support this kind of target " .. cfg.kind)
		end

		local deps = table.join(final_dependency, extrafiles, prelink_dependency)
		ninja.add_build(cfg, cfg_output, extra_outputs, command_rule, table.join(objfiles, libs), {}, deps, {})
		outputs = { cfg_output }
	else
		local handler = ninja.handlers[cfg.language]
		if not handler then
			p.error('expected registered ninja handler action for target ' .. cfg.language)
		end
		outputs = handler.target_rules(cfg, toolset)
	end

	p.outln('')
	if #cfg.postbuildcommands > 0 or cfg.postbuildmessage then
		ninja.add_build(cfg, key, {}, 'phony', { 'postbuild_' .. get_key(cfg) }, {}, {}, {})
	else
		ninja.add_build(cfg, key, {}, 'phony', outputs, {}, {}, {})
	end
	p.outln('')

	path.getDefaultSeparator = oldGetDefaultSeparator
end

-- return name of output binary relative to build folder
function ninja.outputFilename(cfg)
	return project.getrelative(cfg.workspace, cfg.buildtarget.directory) .. '/' .. cfg.buildtarget.name
end

-- return name of build file for configuration
function ninja.projectCfgFilename(cfg, relative)
	if relative ~= nil then
		relative = project.getrelative(cfg.workspace, cfg.location) .. '/'
	else
		relative = ''
	end
	return relative .. get_key(cfg, cfg.project.filename) .. '.ninja'
end

-- check if string starts with string
function ninja.startsWith(str, starts)
	return str:sub(0, starts:len()) == starts
end

-- check if string ends with string
function ninja.endsWith(str, ends)
	return str:sub(-ends:len()) == ends
end

-- generate all build files for every project configuration
function ninja.generateProject(prj)
	if not ninja.can_generate(prj) then
		return
	end
	for cfg in project.eachconfig(prj) do
		p.generate(cfg, ninja.projectCfgFilename(cfg), ninja.generateProjectCfg)
	end
end

include('_preload.lua')

return ninja
