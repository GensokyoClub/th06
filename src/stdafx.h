#pragma once

#define _WIN32_WINNT 0x0500
#define DIRECTINPUT_VERSION 0x0800

#include "BuildVersion.hpp"

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <d3d8.h>
#if BUILT_BEFORE(013) && !TRIALBUILD
// Through 1.02c, DebugPrint.hpp came before <d3dx8.h>. The precompiled header emits its inlines as a stream paced
// by code volume, and <d3dx8.h>'s inline math is most of it, so every object reaches DebugPrint early and emits
// its copy right after its first caller. Later builds include it after Global.hpp (below); that include is then
// a no-op here.
#include "DebugPrint.hpp"
#endif
#include <d3dx8.h>
#include <dinput.h>
#include <dsound.h>

#include "Global.hpp"
// Before 1.02g, EclManager's DebugPrint copy follows its first caller instead of trailing the object, as it does
// when the function is not in the precompiled header (configure.py). Why ZUN's build differed is not known.
#ifndef PCH_WITHOUT_DEBUGPRINT
#include "DebugPrint.hpp"
#endif
#include "Supervisor.hpp"
#include "ZunTimer.hpp"
#include "AnmManager.hpp"
#include "Gui.hpp"
#include "GameManager.hpp"
#include "SoundPlayer.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
