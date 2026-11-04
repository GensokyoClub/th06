#pragma once

#define _WIN32_WINNT 0x0500
#define DIRECTINPUT_VERSION 0x0800

#include "BuildVersion.hpp"

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <dinput.h>
#include <dsound.h>

#include "Global.hpp"
#include "Supervisor.hpp"
#include "AnmManager.hpp"
#include "Gui.hpp"
#include "GameManager.hpp"
#include "SoundPlayer.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
