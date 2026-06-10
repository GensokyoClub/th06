# Export Compile Commands - ECC
Module implementing support for [JSON Compilation Database Format
Specification](https://clang.llvm.org/docs/JSONCompilationDatabase.html).

This is an alternative to
[tarruda's](https://github.com/tarruda/premake-export-compile-commands) module,
which does one simple thing - generates one `compile_commands.json` file for
your project.

Tested with clangd-13.

## Requirements
Premake 5.0.0 or later.

## How to use
Download premake-ecc and place it near your premake5.lua script. Then require it
from your premake script

```lua
require "ecc/ecc"
```

After you simply can call:

```
premake5 ecc
```

Moldule will generate **one** `compile_commands.json` file near your main
premake script. During generation it will use the default config (the first one
you have specified in script). If you want to select specific config just pass
it's name with command line option:

```
premake5 --config=release ecc
```

Careful! `config` option case sensitive! If there is no config passed via
command line, module will choose the default one. 

Note: if you want to embed this module into your premake build follow
the [manual](https://premake.github.io/docs/Embedding-Modules/)

## Future plans
- Add unit tests

## Alternatives
- [export-compile-commands](https://github.com/tarruda/premake-export-compile-commands)
- [bear](https://github.com/rizsotto/Bear)
