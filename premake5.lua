workspace "th06"
  configurations { "Debug", "Release" }
  location "build"

newoption {
   trigger = "no-asoundlib",
   description = "Disables use of the ALSA MIDI interface on Linux. Results in a build without MIDI support."
}

project "th06"
  language "C++"
  cppdialect "C++20"
  targetname "th06"
  targetdir "."
  objdir "obj"

  files {
    "src/AnmManager.cpp",
    "src/AsciiManager.cpp",
    "src/BombData.cpp",
    "src/BulletData.cpp",
    "src/BulletManager.cpp",
    "src/Chain.cpp",
    "src/Controller.cpp",
    "src/EclManager.cpp",
    "src/EffectManager.cpp",
    "src/Ending.cpp",
    "src/EnemyEclInstr.cpp",
    "src/EnemyManager.cpp",
    "src/FileSystem.cpp",
    "src/GameErrorContext.cpp",
    "src/GameManager.cpp",
    "src/GameWindow.cpp",
    "src/GLFunc.cpp",
    "src/Gui.cpp",
    "src/ItemManager.cpp",
    "src/main.cpp",
    "src/MainMenu.cpp",
    "src/MidiOutput.cpp",
    "src/MusicRoom.cpp",
    "src/Player.cpp",
    "src/ReplayManager.cpp",
    "src/ResultScreen.cpp",
    "src/Rng.cpp",
    "src/ScreenEffect.cpp",
    "src/SoundPlayer.cpp",
    "src/Stage.cpp",
    "src/Supervisor.cpp",
    "src/TextHelper.cpp",
    "src/utils.cpp",
    "src/ZunTimer.cpp",
    "src/pbg3/FileAbstraction.cpp",
    "src/pbg3/IPbg3Parser.cpp",
    "src/pbg3/Pbg3Archive.cpp",
    "src/pbg3/Pbg3Parser.cpp",
    -- keep headers visible
    "src/**.hpp"
  }

  includedirs { "src" }

  filter "toolset:gcc"   buildoptions { "-Wall", "-Wextra", "-Wpedantic" }
  filter "toolset:clang" buildoptions { "-Wall", "-Wextra", "-Wpedantic", "-Wno-gnu-anonymous-struct" }
  filter {}

  kind "WindowedApp"

  if os.target() == "windows" then
    files { "src/midi/MidiWin32.cpp" }
    links { "winmm" }
  elseif os.target() == "linux" and _OPTIONS["no-asoundlib"] == nil then
    defines { "LIBASOUND_MIDI_SUPPORT" }
    files { "src/midi/MidiAlsa.cpp" }
    links { "asound" }
  else
    print("Note: Build won't include MIDI support")
    files { "src/midi/MidiDefault.cpp" }
  end

  filter "system:linux"
    local sdl2_cflags = os.outputof("sdl2-config --cflags") or ""
    local sdl2_libs   = os.outputof("sdl2-config --libs")   or ""
    if #sdl2_cflags > 0 then buildoptions { sdl2_cflags } end
    if #sdl2_libs   > 0 then linkoptions  { sdl2_libs }   end
    links { "SDL2_image", "SDL2_ttf", "m" }
  filter {}

  filter "system:windows"
    defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN" }
  filter {}

  filter { "system:windows", "action:vs*" }
    warnings "Extra"
    links { "SDL2", "SDL2main", "SDL2_image", "SDL2_ttf", "iconv" }

    local SDL2_DIR       = os.getenv("SDL2_DIR")
    local SDL2_IMAGE_DIR = os.getenv("SDL2_IMAGE_DIR")
    local SDL2_TTF_DIR   = os.getenv("SDL2_TTF_DIR")

    if SDL2_DIR then
      includedirs { SDL2_DIR .. "/include" }
      filter { "architecture:x86_64" } libdirs { SDL2_DIR .. "/lib/x64" }
      filter { "architecture:x86"    } libdirs { SDL2_DIR .. "/lib/x86" }
      filter {}
    end

    if SDL2_IMAGE_DIR then
      includedirs { SDL2_IMAGE_DIR .. "/include" }
      filter { "architecture:x86_64" } libdirs { SDL2_IMAGE_DIR .. "/lib/x64" }
      filter { "architecture:x86"    } libdirs { SDL2_IMAGE_DIR .. "/lib/x86" }
      filter {}
    end

    if SDL2_TTF_DIR then
      includedirs { SDL2_TTF_DIR .. "/include" }
      filter { "architecture:x86_64" } libdirs { SDL2_TTF_DIR .. "/lib/x64" }
      filter { "architecture:x86"    } libdirs { SDL2_TTF_DIR .. "/lib/x86" }
      filter {}
    end
  filter {}

  filter { "system:windows", "action:not vs*" }
    local pc_cflags = os.outputof("pkg-config --cflags sdl2 SDL2_image SDL2_ttf iconv") or ""
    local pc_libs   = os.outputof("pkg-config --libs   sdl2 SDL2_image SDL2_ttf iconv") or ""
    if #pc_cflags > 0 then buildoptions { pc_cflags } end
    if #pc_libs   > 0 then linkoptions  { pc_libs }   end
  filter {}

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"
    optimize "Off"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "Speed"
    symbols "Off"

  filter {}
