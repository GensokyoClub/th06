# 東方紅魔郷　～ the Embodiment of Scarlet Devil [Multiplayer]

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="resources/progress_dark.svg">
  <img alt="Decomp Progress" src="resources/progress.svg">
</picture>

[![Discord][discord-badge]][discord] <- click here to join discord server.

[discord]: https://discord.gg/VyGwAjrh9a
[discord-badge]: https://img.shields.io/discord/1147558514840064030?color=%237289DA&logo=discord&logoColor=%23FFFFFF

This project aims to make multiplayer from reconstructed source code of [Touhou Koumakyou ~ the Embodiment of Scarlet Devil 1.02h](https://en.touhouwiki.net/wiki/Embodiment_of_Scarlet_Devil) by Team Shanghai Alice.

## Targets 2
-netplay (done?) <br>
-player 2 custom shot type (done?)<br>
-new demo gameplay (demo play is banned currently...)<br>

## NetPlay
# Multiplayer Guide

### 1. Host Side
* Simply click **as host**.
* Wait for the guest to respond.

### 2. Guest Side
* Enter the host's IP address.
* Click **as guest**.
* Wait for the connection to be established.


### Network and Port Settings
* **Standard Scenario:** If you are not using network tunneling/penetration, you usually don't need to change the **host port**. If you must change it, ensure the guest's **host port** matches the host's **listen port**.
* **Network Tunneling (Intranet Penetration):** 
    * Select **UDP** as the tunnel type.
    * **Host:** In your tunneling software, set the local port to the **listen port**.
    * **Guest:** Set **host port** to the public port provided by the tunneling service.
    * **Recommendation:** When using tunneling, it is recommended to set **target delay >= 2**, otherwise the game will be very laggy.
    * Press M or N in-game to increase/decrease the target delay.


### After Connection
* The host can choose whether to be **1P** and set the **target delay**.
* **Note:** A higher target delay can better tolerate network latency, but the input lag for controls will also increase.
* Click **start game** to begin.


### Troubleshooting and Additional Notes
* **Reconnection:** If the connection drops in-game, there is a 5-second timeout. If it doesn't reconnect within this time, you can try returning to the main menu (at the Reimu head screen) to reconnect (though this may not always work).
* **Background Music:** The game does not include WAV format BGM. Please add it yourself if needed.
* **Plugins and Testing:** `d3d8.dll` is a d3d8-to-9 conversion plugin. If you wish to use dgVoodoo (dgv), please add it yourself. It is recommended to select **Start Game (local)** to test performance before attempting multiplayer.


### P2 Color Swapping
* After version 3.6, you can freely choose P1/P2 characters. If you want to change colors at this point, you need to unpack `TOLOL_CM.DAT`, then replace the following files: `player00b.png/player01b.png/player00b_a.png/player01b_a.png`.
* The description files for these are stored in `player00b.anm` and `player01b.anm` respectively.
* hitbox is added in 3.7.1, if want to change, unpack `TOTOL_CM.dat`，including `hitbox.png`,`hitbox_a.png`,`mod_anm.anm`'.

## 3.8
- Adjusted score rewards (extend thresholds) to be more balanced (10, 20, 40, 60, 100, 150, 200, 250M).
- When one is not shooting and overlaps with the other, holding the Focus button for 1.5s will transfer a life to the other player.
- Disabled replay playback in multiplayer mode (replays can still be played in single-player). The connection will automatically disconnect upon entering the score screen and will only automatically reconnect once you return to the title screen (not the title menu).
- Fixed a bug where a Game Over would be triggered if either player missed while at 0 lives.
- Fixed GUI issues when entering player signatures.

### 3.7.1 Updates
- hitbox when focusing
- fix a 0.5px texture offset

### 3.7 Updates
- MarisaB is capable to shoot when another player is bombing
- ESC+R added
- initial resource is changed to 2-3
- resources for P1/P2 is calculated separately
- automatically hide another player if too close
- add more score bonus life (10,20,30,40,50,60,80,100,125,150,200,250M)

### 3.6.1 Updates
- "P1 as host" is removed since P1/P2 character can be selected in game.
- Code of timeout is changed to make it more stable.

### 3.6 Updates

- Added P1/P2 character selection and color swapping.


### 3.5 Updates

- Added Esc+Q shortcut.
- Pressing F2, F3, or F4 will randomly drop Life/Bomb/Power items on the screen.
- Cross-version multiplayer is no longer supported.
- Modified item collection logic; items are now collected by the player closest to them.
- Removed the frame damage cap and the Full Power item collection mechanism.
- Config file = `th06e.cfg`.
- Press M/N to increase/decrease delay.

---

### Troubleshooting & Additional Info
* **Disconnection:** There is a 5-second timeout if the connection drops. If it doesn't reconnect within this time, you can try returning to **"start game"** in the main menu to reconnect (though this is not guaranteed to work).

## Targets

-2 player controls (done)<br>
-different character (done)<br>
-replay (done)<br>
-provoke enemy by player (done)<br>

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
