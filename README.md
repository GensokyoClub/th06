[![Discord][discord-badge]][discord] <- click here to join discord server.

[discord]: https://discord.gg/VyGwAjrh9a
[discord-badge]: https://img.shields.io/discord/1147558514840064030?color=%237289DA&logo=discord&logoColor=%23FFFFFF

This is the readme for the remastered fork of EoSD. Unless you know what you're doing, this probably isn't what you're looking for, as remastered requires some asset modifications to run. For a drop-in compatible portable replacement of EoSD see the [portable branch](https://github.com/GensokyoClub/th06/tree/portable). For the readme of the decomp project, see [here](https://github.com/GensokyoClub/th06/blob/master/README.md).

EoSD-remastered is a port of Touhou 6 using SDL2 and OpenGL (with a more general renderer abstraction layer hopefully on the way).
This enables theoretical portability to any system supported by SDL2, with Linux and Windows in particular being known to work.
Builds for OS X, the BSDs, and other Unices are also almost certainly possible, but may require some slight modifications to the build system.

### Platform Requirements

- SDL2, SDL2-image, and SDL2-ttf support
- C++20 standard library support
- A little endian architecture (though big endian support is currently being worked on)
- OpenGL ES 1.1, OpenGL 1.3, or GL ES 2.0 / WebGL support

### Dependencies

EoSD-remastered has the following dependencies:

- `SDL2`
- `SDL2_image`
- `SDL2_ttf`
- `libasound` (Optional and Linux-only, enables MIDI support. This will almost always be present as part of a desktop distro.)

In addition, building uses [`premake5`](https://premake.github.io/download) and a compiler that supports C++20.

#### Building

In the repository root directory, run `premake5` with the desired build system as an argument (a list can be seen by running `premake5 --help`).
This will output the build files to the `build` directory, and then compilation may be done with the desired build system.

##### Build Options (Use with Premake Invocation)
`--no-asoundlib`: On Linux, doesn't build MIDI support. Removes libasound as a dev and runtime dependency
`--use-c23-embed`: Uses `#embed` for resource inclusion instead of a lua script in the Premake file.

##### Build Example (Debian-based Linux)

Obtain dependencies:

`sudo apt install build-essential libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libasound2-dev`

Generate makefile:

`premake5 gmake`

Compile:

`cd build && make -j16`

### Use

EoSD-remastered is NOT a drop-in replacement for EoSD. Currently it requires unpacked archive files and merging of some of the texture spritesheets. There is a script to apply these automatically, `scripts/configure_assets.sh`, but it requires thtk and has dubious portability.
You will also need to add a font to your game directory with the filename `msgothic.ttc`.
This may be the actual MS Gothic, taken from a Windows machine, or a compatible font such as Kochi Gothic.
English and other patches, static or thcrap, do not currently work.
A Japanese locale is not required.

# Decomp Credits

We would like to extend our thanks to the following individuals for their
invaluable contributions:

- @EstexNT for porting the [`var_order` pragma](scripts/pragma_var_order.cpp) to
  MSVC7.
