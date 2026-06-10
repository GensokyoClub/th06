#pragma once

#include "inttypes.hpp"

/* COLORS */
#define COLOR_BLACK 0xff000000
#define COLOR_GREY 0xff808080
#define COLOR_DARK_GREY 0xff505050
#define COLOR_ASHEN_GREY 0xff606060
#define COLOR_WHITE 0xffffffff
#define COLOR_TRANSPARENT_WHITE 0x80ffffff
#define COLOR_RED 0xffff0000
#define COLOR_YELLOW 0xffffff00
#define COLOR_LIGHT_RED 0xffff8080
#define COLOR_PASTEL_RED 0xffff6060
#define COLOR_SUNSHINEYELLOW 0xffffff40
#define COLOR_PINK 0xffffe0e0
#define COLOR_LIGHTCYAN 0xffe0ffff
#define COLOR_LAVENDER 0xffe0e0ff
#define COLOR_LIGHTBLUE 0xffd0d0ff
#define COLOR_LIGHTYELLOW 0xffffff80
#define COLOR_PALEBLUE 0xffc0b0ff
#define COLOR_NEONBLUE 0xff4040ff
#define COLOR_DEEPBLUE 0xff3030ff
#define COLOR_GUI_1 0xffe8f0ff
#define COLOR_GUI_2 0xffffe8f0

#define COLOR_RGB_MASK 0x00FFFFFF
#define COLOR_ALPHA_MASK 0xFF000000
#define COLOR_RGB(color) ((color) & COLOR_RGB_MASK)
#define COLOR_ALPHA(color) (((color) & COLOR_ALPHA_MASK) >> 24)
#define COLOR_SET_ALPHA(color, alpha) (((alpha) << 24) | COLOR_RGB(color))
#define COLOR_SET_ALPHA2(color, alpha) (COLOR_RGB(color) | (((alpha) & 0xff) << 24))
#define COLOR_SET_ALPHA3(color, alpha) (COLOR_RGB(color) | ((alpha) << 24))
#define COLOR_COMBINE_ALPHA(color, alpha) (((alpha) & COLOR_ALPHA_MASK) | COLOR_RGB(color))

#define COLOR_MENU_ACTIVE_BACKGROUND 0x40000000
// TODO: find a better name for this color
#define COLOR_START_MENU_ITEM_INACTIVE 0x80300000

#define COLOR_KEYBOARD_KEY_HIGHLIGHT 0xffffffc0
#define COLOR_KEYBOARD_KEY_NORMAL 0x60c0c0c0

#define COLOR_MENU_ITEM_HIGHLIGHT 0x800000ff
#define COLOR_MENU_ITEM_DEFAULT 0x80202050

// BGR
#define COLOR_END_TEXT_SHADOW 0xc0d0d0
#define COLOR_MUSIC_ROOM_SONG_TITLE_TEXT 0xc0e0ff
#define COLOR_MUSIC_ROOM_SONG_TITLE_SHADOW 0x302080
#define COLOR_MUSIC_ROOM_SONG_DESC_TEXT 0xffe0c0
#define COLOR_MUSIC_ROOM_SONG_DESC_SHADOW 0x300000

// TODO: The following assumes little endian
#define COLOR_RED_BYTE_IDX 0
#define COLOR_GREEN_BYTE_IDX 1
#define COLOR_BLUE_BYTE_IDX 2
#define COLOR_ALPHA_BYTE_IDX 3

#define COLOR_GET_COMPONENT(color, component) (((u8 *)&(color))[(component)])
#define COLOR_SET_COMPONENT(color, component, value) ((u8 *)&(color))[(component)] = (value);

typedef u32 ZunColor;
