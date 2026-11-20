# ANM documentation
This document explains the ANM file format and scripting language
for *Touhou 6: Embodiment of Scarlet Devil*.

## File format
### File structure

An ANM file is generally structured like this:

- File header
- Sprite table
- Script table
- Texture paths
- Sprite data
- Script data

### File header (`AnmRawEntry`)
| Offset (bytes) | Size (bytes) | Member | Description |
| -------------- | ------------ | ------ | ----------- |
| 0x0 | 0x4 | numSprites | The number of sprites. |
| 0x4 | 0x4 | numScripts | The number of scripts. |
| 0x8 | 0x4 | textureIdx | The runtime texture index. The file's value is not used, but is set to the ANM slot by the engine when loaded. |
| 0xc  | 0x4 | width | The runtime width. [^1] |
| 0x10 | 0x4 | height | The runtime height. [^1] |
| 0x14 | 0x4 | format | The runtime format. Refer to the format table below. |
| 0x18 | 0x4 | colorKey | |
| 0x1c | 0x4 | texturePathOffset | The offset to the file path of the main texture. [^2] |
| 0x20 | 0x4 | spriteIdxOffset | The runtime script/sprite offset. The file's value is not used, but is set to where the engine loads the sprites in the global sprite array. |
| 0x24 | 0x4 | alphaTexturePathOffset | The offset to the file path of the alpha texture. [^2]|
| 0x28 | 0x18 | | Unused. Might be padding to align the header to 64 bytes. |

[^1]: If this does not match the texture, then the texture will be scaled to this width/height.
[^2]: The file path is located usually after the sprite and script table, but this is not mandatory.

### Texture format table (`g_TextureFormatD3D8Mapping`)
| Value | D3D8 format |
| ----- | ----------- |
| 0 | D3DFMT_UNKNOWN[^3] |
| 1 | D3DFMT_A8R8G8B8 |
| 2 | D3DFMT_A1R5G5B5 |
| 3 | D3DFMT_R5G6B5 |
| 4 | D3DFMT_R8G8B8 |
| 5 | D3DFMT_A4R4G4B4 |

[^3]: Wildcard texture.
### Sprite table
Each entry is a 32-bit offset to a sprite.

### Script table
Each entry follows this structure, in the codebase, this is struct `AnmRawScript`:
| Offset (bytes) | Size (bytes) | Member | Description |
| -------------- | ------------ | ------ | ----------- |
| 0x0 | 0x4 | id | The script ID. This is also the offset where the engine loads the script in the global script array, offset by the runtime spriteIdxOffset field. |
| 0x4 | 0x4 | offset | The offset to the first instruction in the script. |

### Sprite data
In the codebase, this is `AnmRawSprite`.
| Offset (bytes) | Size (bytes) | Member | Description |
| -------------- | ------------ | ------ | ----------- |
| 0x0 | 0x4 | id | The sprite ID. This is also the offset where the engine loads the sprite in the global sprite array, offset by the runtime spriteIdxOffset field. |
| 0x4 | 0x4 | x | X position of the sprite. |
| 0x8 | 0x4 | y | Y position of the sprite. |
| 0xc | 0x4 | width | Width of the sprite. |
| 0x10 | 0x4 | height | Height of the sprite. |

The coordinates are all **floating point numbers**, and originate from
the **top-left**.

### Script data
An ANM instruction (`AnmRawInstr`) has this structure:
| Offset (bytes) | Size (bytes) | Member | Description |
| -------------- | ------------ | ------ | ----------- |
| 0x0 | 0x2 | time | The time value. |
| 0x2 | 0x1 | opcode | The opcode. |
| 0x3 | 0x1 | argsSize | The size of the arguments |

## Script runtime
In Touhou 6, the ANM system is used to render most of the game. Each
sprite, known as an `AnmVm`, can run a script, with each instruction
changing the state of the sprite (hence why it is known as an **ANM**
**v**irtual **m**achine). The function which runs these instructions is
`AnmManager::ExecuteScript` (located at `0x433960` in version 1.02h).

To advance an instruction, add the size of an ANM instruction (4 bytes)
to the argSize field.

An `AnmVm` has an internal clock, `scriptTime`, which is incremented
every time this function is run on an `AnmVm`. If the current
instruction's time is less than the value of `scriptTime`, then the
script will temporarily stop running, until the `scriptTime` value is
greater or equal to the current instruction's time value.

### Opcodes
The following table are the opcodes for instructions. The names are
from zero318's ANM mapfile.
| Signature | Number | Description |
| ---- | ------ | ----------- |
| anm_delete() | 0 | Makes the sprite invisible, and terminates script execution. |
| set_sprite(int sprite) | 1 | Sets the sprite to `sprite` |
| scale(float x, float y) | 2 | Sets the scale to `x` and `y`. |
| alpha(int a) | 3 | Sets the alpha component of the `AnmVm`'s color. |
| color(int color) | 4 | Set the `AnmVm`'s color. This does not affect the alpha channel. |
