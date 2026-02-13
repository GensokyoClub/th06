# 東方紅魔郷　～ the Embodiment of Scarlet Devil [Multiplayer]

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="resources/progress_dark.svg">
  <img alt="Decomp Progress" src="resources/progress.svg">
</picture>

[![Discord][discord-badge]][discord] <- click here to join discord server.

[discord]: https://discord.gg/VyGwAjrh9a
[discord-badge]: https://img.shields.io/discord/1147558514840064030?color=%237289DA&logo=discord&logoColor=%23FFFFFF

This project aims to make multiplayer from reconstructed source code of [Touhou Koumakyou ~ the Embodiment of Scarlet Devil 1.02h](https://en.touhouwiki.net/wiki/Embodiment_of_Scarlet_Devil) by Team Shanghai Alice.

## Targets

-2 player controls (done)<br>
-different character (done)<br>
-replay (done)<br>
-provoke enemy by player (done)<br>
-player 2 custom shot type (todo)<br>
-netplay (todo)<br>
-new demo gameplay (todo)<br>

## Installation

### Executable

This project requires the original `東方紅魔郷.exe` version 1.02h (SHA256 hashsum 9f76483c46256804792399296619c1274363c31cd8f1775fafb55106fb852245, you can check hashsum on windows with command `certutil -hashfile <path-to-your-file> SHA256`.)

Copy `東方紅魔郷.exe` to `resources/game.exe`.

### Dependencies

The build system has the following package requirements:

- `python3` >= 3.4
- `msiextract` (On linux/macos only)
- `wine` (on linux/macos only, prefer CrossOver on macOS to avoid possible CL.EXE heap issues)
- `aria2c` (optional, allows for torrent downloads, will automatically install on Windows if selected.)

The rest of the build system is constructed out of Visual Studio 2002 and DirectX 8.0 from the Web Archive.

#### Configure devenv

This will download and install compiler, libraries, and other tools.

If you are on windows, and for some reason want to download dependencies manually,
run this command to get the list of files to download:

```
python scripts/create_devenv.py scripts/dls scripts/prefix --no-download
```

But if you want everything to be downloaded automatically, run it like this instead:

```
python scripts/create_devenv.py scripts/dls scripts/prefix
```

And if you want to use torrent to download those dependencies, use this:

```
python scripts/create_devenv.py scripts/dls scripts/prefix --torrent
```

On linux and mac, run the following script:
```bash
# NOTE: On macOS if you use CrossOver.
# export WINE=<CrossOverPath>/wine
./scripts/create_th06_prefix
```

#### Building

Run the following script:

```
python3 ./scripts/build.py
```

This will automatically generate a ninja build script `build.ninja`, and run
ninja on it.

## Contributing

### Reverse Engineering

You can find an XML export of our Ghidra RE in the companion repository
[th06-re], in the `xml` branch. This repo is updated nightly through
[`scripts/export_ghidra_database.py`], and its history matches the checkin
history from our team's Ghidra Server.

If you wish to help us in our Reverse Engineering effort, please contact
@roblabla on discord so we can give you an account on the Ghidra Server.

### Reimplementation

The easiest way to work on the reimplementation is through the use of
[`objdiff`](https://github.com/encounter/objdiff). Here's how to get started:

1. First, follow the instruction above to get a devenv setup.
1. Copy the original `東方紅魔郷.exe` file (version 1.02h) to the
   `resources/` folder, and rename it into `game.exe`. This will be used as the source to compare the
   reimplementations against.
1. Download the latest version of objdiff.
1. Run `python3 scripts/export_ghidra_objs.py --import-csv`. This will extract
   from `resources/game.exe` the object files that objdiff can compare against.
1. Finally, run objdiff and open the th06 project.

#### Choosing a function to decompile

The easiest is to look at the `config/stubbed.csv` files. Those are all
functions that are automatically stubbed out. You should pick one of them, open
the associated object file in objdiff, and click on the function of interest.

Then, open the correct `cpp` file, copy/paste the declaration, and start
hacking! It may be useful to take the ghidra decompiler output as a base. You
can find this output in the [th06-re] repository.

# Credits

We would like to extend our thanks to the following individuals for their
invaluable contributions:

- @EstexNT for porting the [`var_order` pragma](scripts/pragma_var_order.cpp) to
  MSVC7.

[th06-re]: https://github.com/happyhavoc/th06-re
