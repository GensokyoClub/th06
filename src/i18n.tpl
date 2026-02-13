#pragma once

#define TH_LANG TH_EN

// JP
#define TH_JP_CONFIG_FILE "東方紅魔郷.cfg"
#define TH_JP_WINDOW_TITLE "東方紅魔郷　〜 the Embodiment of Scarlet Devil"
#define TH_JP_DBG_MAINMENU_VRAM "Debug : title 開始 VRAM = %d\n"
#define TH_JP_DBG_RESULTSCREEN_COUNAT "counat = %d\n"
#define TH_JP_ERR_ALREADY_RUNNING "二つは起動できません\n"
#define TH_JP_ERR_D3D_COULD_NOT_CREATE_OBJ "Direct3D オブジェクトは何故か作成出来なかった\n"
#define TH_JP_ERR_LOGGER_START "東方動作記録 --------------------------------------------- \n"
#define TH_JP_ERR_LOGGER_END "---------------------------------------------------------- \n"
#define TH_JP_ERR_NO_PAD_FOUND "使えるパッドが存在しないようです、残念\n"
#define TH_JP_ERR_OPTION_CHANGED_RESTART "再起動を要するオプションが変更されたので再起動します\n"
#define TH_JP_ERR_SCREEN_INIT_32BITS "初回起動、画面を 32Bits で初期化しました\n"
#define TH_JP_ERR_SCREEN_INIT_16BITS "初回起動、画面を 16Bits で初期化しました\n"
#define TH_JP_ERR_SET_REFRESH_RATE_60HZ "リフレッシュレートを60Hzに変更します\n"
#define TH_JP_ERR_TL_HAL_UNAVAILABLE "T&L HAL は使用できないようです\n"
#define TH_JP_ERR_HAL_UNAVAILABLE "HAL も使用できないようです\n"
#define TH_JP_ERR_D3D_INIT_FAILED "Direct3D の初期化に失敗、これではゲームは出来ません\n"
#define TH_JP_ERR_BACKBUFFER_NONLOCKED "バックバッファをロック不可能にしてみます\n"
#define TH_JP_ERR_CANT_CHANGE_REFRESH_RATE_FORCE_VSYNC "リフレッシュレートが変更できません、vsync 非同期に変更します\n"
#define TH_JP_USING_REF_MODE "REF で動作しますが、重すぎて恐らくゲームになりません...\n"
#define TH_JP_USING_HAL_MODE "HAL で動作します\n"
#define TH_JP_USING_TL_HAL_MODE "T&L HAL で動作しま〜す\n"
#define TH_JP_ERR_NO_SUPPORT_FOR_D3DTEXOPCAPS_ADD                                                                      \
    "D3DTEXOPCAPS_ADD をサポートしていません、色加算エミュレートモードで動作します\n"
#define TH_JP_ERR_CANT_FORCE_60FPS_NO_ASYNC_FLIP                                                                       \
    "ビデオカードが非同期フリップをサポートしていません、Force60Frameで動作できません\n"
#define TH_JP_ERR_D3DFMT_A8R8G8B8_UNSUPPORTED "D3DFMT_A8R8G8B8 をサポートしていません、減色モードで動作します\n"
#define TH_JP_ERR_CONFIG_NOT_FOUND "コンフィグデータが見つからないので初期化しました\n"
#define TH_JP_ERR_CONFIG_CORRUPTED "コンフィグデータが破壊されていたので再初期化しました\n"
#define TH_JP_ERR_NO_WAVE_FILE "wave データが無いので、midi にします\n"
#define TH_JP_ERR_NOT_A_WAV_FILE "Wav ファイルじゃない? %s\n"
#define TH_JP_ERR_NO_VERTEX_BUFFER "頂点バッファの使用を抑制します\n"
#define TH_JP_ERR_NO_FOG "フォグの使用を抑制します\n"
#define TH_JP_ERR_USE_16BIT_TEXTURES "16Bit のテクスチャの使用を強制します\n"
#define TH_JP_ERR_FORCE_BACKBUFFER_CLEAR "バックバッファの消去を強制します\n"
#define TH_JP_ERR_DONT_RENDER_ITEMS "ゲーム周りのアイテムの描画を抑制します\n"
#define TH_JP_ERR_NO_GOURAUD_SHADING "グーローシェーディングを抑制します\n"
#define TH_JP_ERR_NO_DEPTH_TESTING "デプステストを抑制します\n"
#define TH_JP_ERR_FORCE_60FPS_MODE "60フレーム強制モードにします\n"
#define TH_JP_ERR_NO_TEXTURE_COLOR_COMPOSITING "テクスチャの色合成を抑制します\n"
#define TH_JP_ERR_LAUNCH_WINDOWED "ウィンドウモードで起動します\n"
#define TH_JP_ERR_FORCE_REFERENCE_RASTERIZER "リファレンスラスタライザを強制しますn"
#define TH_JP_ERR_DO_NOT_USE_DIRECTINPUT "パッド、キーボードの入力に DirectInput を使用しません\n"
#define TH_JP_ERR_DIRECTINPUT_NOT_AVAILABLE "DirectInput が使用できません\n"
#define TH_JP_ERR_DIRECTINPUT_SETDATAFORMAT_NOT_AVAILABLE "DirectInput SetDataFormat が使用できません\n"
#define TH_JP_ERR_DIRECTINPUT_SETCOOPERATIVELEVEL_NOT_AVAILABLE "DirectInput SetCooperativeLevel が使用できません\n"
#define TH_JP_ERR_DIRECTINPUT_INITIALIZED "DirectInput は正常に初期化されました\n"
#define TH_JP_ERR_PAD_FOUND "有効なパッドを発見しました\n"
#define TH_JP_ERR_FILE_CANNOT_BE_EXPORTED "ファイルが書き出せません %s\n"
#define TH_JP_ERR_FOLDER_HAS_WRITE_PROTECT_OR_DISK_FULL                                                                \
    "フォルダが書込み禁止属性になっているか、ディスクがいっぱいいっぱいになってませんか？\n"
#define TH_JP_ERR_ASCIIMANAGER_INIT_FAILED "error : 文字の初期化に失敗しました\n"
#define TH_JP_ERR_WRONG_DATA_VERSION "error : データのバージョンが違います\n"
#define TH_JP_ERR_CANNOT_BE_LOADED "%sが読み込めないです。\n"
#define TH_JP_ERR_ANMMANAGER_SPRITE_CORRUPTED "スプライトアニメ %s が読み込めません。データが失われてるか壊れています\n"
#define TH_JP_ERR_ANMMANAGER_TEXTURE_CORRUPTED "テクスチャ %s が読み込めません。データが失われてるか壊れています\n"
#define TH_JP_ERR_ANMMANAGER_UNK_TEX_FORMAT "error : イメージがαを持っていません\n"
#define TH_JP_ERR_ECLMANAGER_ENEMY_DATA_CORRUPT "敵データの読み込みに失敗しました、データが壊れてるか失われています\n"
#define TH_JP_ERR_ENDING_END_FILE_CORRUPTED "error : エンディングファイルが読み込めない、ファイルが破壊されています\n"
#define TH_JP_ERR_MAINMENU_LOAD_SELECT_SCREEN_FAILED "セレクト画面の読み込みに失敗\n"
#define TH_JP_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER "error : ストリーミング用サウンドバッファを作成出来ませんでした\n"
#define TH_JP_ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT "DirectSound オブジェクトの初期化が失敗したよ\n"
#define TH_JP_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE "error : Sound ファイルが読み込めない データを確認 %s\n"
#define TH_JP_DBG_SOUNDPLAYER_INIT_SUCCESS "DirectSound は正常に初期化されました\n"
#define TH_JP_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_STAGE "error : 背景データの初期化に失敗しました\n"
#define TH_JP_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_PLAYER "error : プレイヤーの初期化に失敗しました\n"
#define TH_JP_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_BULLETMANAGER "error : 敵弾の初期化に失敗しました\n"
#define TH_JP_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ENEMYMANAGER "error : 敵の初期化に失敗しました\n"
#define TH_JP_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ECLMANAGER "error : 敵頭脳の初期化に失敗しました\n"
#define TH_JP_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_EFFECTMANAGER "error : エフェクトの初期化に失敗しました\n"
#define TH_JP_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_GUI "error : 2D表示の初期化に失敗しました\n"
#define TH_JP_ERR_GUI_MSG_FILE_CORRUPTED "error : メッセージファイル %s が読み込めませんでした\n"
#define TH_JP_ERR_STAGE_DATA_CORRUPTED "ステージデータが見つかりません。データが壊れています\n"
#define TH_JP_ERR_MIDI_FAILED_TO_READ_FILE "error : MIDI File が読み込めない %s \n"
#define TH_JP_ERR_MIDI_NOT_LOADED "error : まだMIDIが読み込まれていないのに再生しようとしている\n"

#define TH_JP_REIMU_A_BOMB_NAME "霊符「夢想封印」"
#define TH_JP_REIMU_B_BOMB_NAME "夢符「封魔陣」"
#define TH_JP_MARISA_A_BOMB_NAME "魔符「スターダストレヴァリエ」"
#define TH_JP_MARISA_B_BOMB_NAME "恋符「マスタースパーク」"

#define TH_JP_HAKUREI_REIMU_SPIRIT "博麗 霊夢 (霊)"
#define TH_JP_HAKUREI_REIMU_DREAM "博麗 霊夢 (夢)"
#define TH_JP_KIRISAME_MARISA_DEVIL "霧雨 魔理沙 (魔)"
#define TH_JP_KIRISAME_MARISA_LOVE "霧雨 魔理沙 (恋)"
#define TH_JP_SATSUKI_RIN_FLOWER "冴月 麟 (花)"
#define TH_JP_SATSUKI_RIN_WIND "冴月 麟 (風)"

#define TH_JP_UNKNOWN_SPELLCARD "？？？？？"

#define TH_JP_CM_DAT_FILE "紅魔郷CM.dat"
#define TH_JP_ED_DAT_FILE "紅魔郷ED.DAT"
#define TH_JP_IN_DAT_FILE "紅魔郷IN.dat"
#define TH_JP_MD_DAT_FILE "紅魔郷MD.dat"
#define TH_JP_ST_DAT_FILE "紅魔郷ST.dat"
#define TH_JP_TL_DAT_FILE "紅魔郷TL.dat"

// EN
#define TH_EN_CONFIG_FILE "th06.cfg"
#define TH_EN_WINDOW_TITLE "Touhou Koumakyou ~ the Embodiment of Scarlet Devil"
#define TH_EN_DBG_MAINMENU_VRAM "Debug : title menu VRAM = %d\n"
#define TH_EN_DBG_RESULTSCREEN_COUNAT "counat = %d\n"
#define TH_EN_ERR_ALREADY_RUNNING "Touhou cannot be started\n"
#define TH_EN_ERR_D3D_COULD_NOT_CREATE_OBJ "Direct3D object could not be created for some reason\n"
#define TH_EN_ERR_LOGGER_START "Logger started --------------------------------------------- \n"
#define TH_EN_ERR_LOGGER_END "---------------------------------------------------------- \n"
#define TH_EN_ERR_NO_PAD_FOUND "Unfortunately, there doesn't seem to be a pad that can be used.\n"
#define TH_EN_ERR_OPTION_CHANGED_RESTART "An option that requires a restart has been changed.\n"
#define TH_EN_ERR_SCREEN_INIT_32BITS "First startup, screen initialized with 32Bits.\n"
#define TH_EN_ERR_SCREEN_INIT_16BITS "First startup, screen initialized with 16Bits.\n"
#define TH_EN_ERR_SET_REFRESH_RATE_60HZ "Setting the refresh rate to 60Hz.\n"
#define TH_EN_ERR_TL_HAL_UNAVAILABLE "T&L HAL does not appear to be available.\n"
#define TH_EN_ERR_HAL_UNAVAILABLE "HAL does not appear to be available either.\n"
#define TH_EN_ERR_D3D_INIT_FAILED "Direct3D initialization failed, the game cannot be played.\n"
#define TH_EN_ERR_BACKBUFFER_NONLOCKED "Retrying to create D3D context without locked backbuffer.\n"
#define TH_EN_ERR_CANT_CHANGE_REFRESH_RATE_FORCE_VSYNC "Unable to set refresh rate, enabling vsync.\n"
#define TH_EN_USING_REF_MODE "Using REF mode, expect heavy lag...\n"
#define TH_EN_USING_HAL_MODE "Using HAL mode.\n"
#define TH_EN_USING_TL_HAL_MODE "Using T&L HAL mode.\n"
#define TH_EN_ERR_NO_SUPPORT_FOR_D3DTEXOPCAPS_ADD                                                                      \
    "Does not support D3DTEXOPCAPS_ADD, operates in color additive emulation mode.\n"
#define TH_EN_ERR_CANT_FORCE_60FPS_NO_ASYNC_FLIP                                                                       \
    "Video card does not support asynchronous flipping, cannot work with Force60Frame.\n"
#define TH_EN_ERR_D3DFMT_A8R8G8B8_UNSUPPORTED "D3DFMT_A8R8G8B8 not supported, operating in reduced color mode.\n"
#define TH_EN_ERR_CONFIG_NOT_FOUND "Config not found, initializing with default values.\n"
#define TH_EN_ERR_CONFIG_CORRUPTED "Config corrupted, reinitializing with default values.\n"
#define TH_EN_ERR_NO_WAVE_FILE "There is no wave data, so I'll make it midi!\n"
#define TH_EN_ERR_NOT_A_WAV_FILE "%s isn't a wav file?"
#define TH_EN_ERR_NO_VERTEX_BUFFER "Suppressing the use of the vertex buffer.\n"
#define TH_EN_ERR_NO_FOG "Suppressing the use of fog.\n"
#define TH_EN_ERR_USE_16BIT_TEXTURES "Enforces the use of 16Bit textures.\n"
#define TH_EN_ERR_FORCE_BACKBUFFER_CLEAR "Force clearing of the back buffer.\n"
#define TH_EN_ERR_DONT_RENDER_ITEMS "Suppress the rendering of items around the game.\n"
#define TH_EN_ERR_NO_GOURAUD_SHADING "Suppress gouraud shading.\n"
#define TH_EN_ERR_NO_DEPTH_TESTING "Suppress depth testing.\n"
#define TH_EN_ERR_FORCE_60FPS_MODE "Force 60FPS mode.\n"
#define TH_EN_ERR_NO_TEXTURE_COLOR_COMPOSITING "Suppress texture color compositing.\n"
#define TH_EN_ERR_LAUNCH_WINDOWED "Launch in windowed mode.\n"
#define TH_EN_ERR_FORCE_REFERENCE_RASTERIZER "Force reference rasterizer.\n"
#define TH_EN_ERR_DO_NOT_USE_DIRECTINPUT "Do not use DirectInput for pad and keyboard input.\n"
#define TH_EN_ERR_DIRECTINPUT_NOT_AVAILABLE "DirectInput is not available.\n"
#define TH_EN_ERR_DIRECTINPUT_SETDATAFORMAT_NOT_AVAILABLE "DirectInput SetDataFormat is not available.\n"
#define TH_EN_ERR_DIRECTINPUT_SETCOOPERATIVELEVEL_NOT_AVAILABLE "DirectInput SetCooperativeLevel is not available.\n"
#define TH_EN_ERR_DIRECTINPUT_INITIALIZED "DirectInput was successfully initialized.\n"
#define TH_EN_ERR_PAD_FOUND "Found a valid pad.\n"
#define TH_EN_ERR_FILE_CANNOT_BE_EXPORTED "File cannot be exported %s.\n"
#define TH_EN_ERR_FOLDER_HAS_WRITE_PROTECT_OR_DISK_FULL "Folder has write protect attribute or disk full?\n"
#define TH_EN_ERR_ASCIIMANAGER_INIT_FAILED "error: AsciiManager initialization failed\n"
#define TH_EN_ERR_WRONG_DATA_VERSION "error: Wrong data version\n"
#define TH_EN_ERR_CANNOT_BE_LOADED "%s cannot be loaded.\n"
#define TH_EN_ERR_ANMMANAGER_SPRITE_CORRUPTED "Unable to load sprite animation %s. Data is lost or corrupted.\n"
#define TH_EN_ERR_ANMMANAGER_TEXTURE_CORRUPTED "Unable to load texture %s. Data is lost or corrupted.\n"
#define TH_EN_ERR_ANMMANAGER_UNK_TEX_FORMAT "error : Image does not have a valid format\n"
#define TH_EN_ERR_ECLMANAGER_ENEMY_DATA_CORRUPT "Failed to load enemy data, data is corrupt or lost.\n"
#define TH_EN_ERR_ENDING_END_FILE_CORRUPTED "error : Ending file cannot be loaded, the file is corrupted.\n"
#define TH_EN_ERR_MAINMENU_LOAD_SELECT_SCREEN_FAILED "Failed to load character/difficulty selection screen\n"
#define TH_EN_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER "error: Could not create sound buffer for music\n"
#define TH_EN_ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT "DirectSound: Failed to initialize object\n"
#define TH_EN_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE "error: Could not load sound file %s\n"
#define TH_EN_DBG_SOUNDPLAYER_INIT_SUCCESS "DirectSound initialized successfully\n"
#define TH_EN_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_STAGE "error : Failed to initialize Stage.\n"
#define TH_EN_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_PLAYER "error : Failed to initialize Player.\n"
#define TH_EN_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_BULLETMANAGER "error : Failed to initialize BulletManager.\n"
#define TH_EN_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ENEMYMANAGER "error : Failed to initialize EnemyManager.\n"
#define TH_EN_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ECLMANAGER "error : Failed to initialize EclManager.\n"
#define TH_EN_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_EFFECTMANAGER "error : Failed to initialize EffectManager.\n"
#define TH_EN_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_GUI "error : Failed to initialize Gui.\n"
#define TH_EN_ERR_GUI_MSG_FILE_CORRUPTED "error : message file %s could not be read.\n"
#define TH_EN_ERR_STAGE_DATA_CORRUPTED "Stage data not found. Data is corrupted.\n"
#define TH_EN_ERR_MIDI_FAILED_TO_READ_FILE "error : MIDI File %s could not be read.\n"
#define TH_EN_ERR_MIDI_NOT_LOADED "error : MIDI not loaded before being playback started.\n"

#define TH_EN_REIMU_A_BOMB_NAME "Spirit Sign \"Dream Seal\""
#define TH_EN_REIMU_B_BOMB_NAME "Dream Sign \"Evil-Sealing Circle\""
#define TH_EN_MARISA_A_BOMB_NAME "Magic Sign \"Stardust Reverie\""
#define TH_EN_MARISA_B_BOMB_NAME "Love Sign \"Master Spark\""

#define TH_EN_HAKUREI_REIMU_SPIRIT "Hakurei Reimu (Spirit)"
#define TH_EN_HAKUREI_REIMU_DREAM "Hakurei Reimu (Dream)"
#define TH_EN_KIRISAME_MARISA_DEVIL "Kirisame Marisa (Devil)"
#define TH_EN_KIRISAME_MARISA_LOVE "Kirisame Marisa (Love)"
#define TH_EN_SATSUKI_RIN_FLOWER "Satsuki Rin (Flower)"
#define TH_EN_SATSUKI_RIN_WIND "Satsuki Rin (Wind)"

#define TH_EN_UNKNOWN_SPELLCARD "??????"

#define TH_EN_CM_DAT_FILE "TOLOL_CM.dat"
#define TH_EN_ED_DAT_FILE "TOLOL_ED.dat"
#define TH_EN_IN_DAT_FILE "TOLOL_IN.dat"
#define TH_EN_MD_DAT_FILE "TOLOL_MD.dat"
#define TH_EN_ST_DAT_FILE "TOLOL_ST.dat"
#define TH_EN_TL_DAT_FILE "TOLOL_TL.dat"
#define TH_CONCAT_HELPER(x, y) x##y

#define TH_MAKE_LANG_STR(lang, id) TH_CONCAT_HELPER(lang, id)

#define TH_CONFIG_FILE TH_MAKE_LANG_STR(TH_LANG, _CONFIG_FILE)
#define TH_WINDOW_TITLE TH_MAKE_LANG_STR(TH_LANG, _WINDOW_TITLE)
#define TH_DBG_MAINMENU_VRAM TH_MAKE_LANG_STR(TH_LANG, _DBG_MAINMENU_VRAM)
#define TH_DBG_RESULTSCREEN_COUNAT TH_MAKE_LANG_STR(TH_LANG, _DBG_RESULTSCREEN_COUNAT)
#define TH_ERR_ALREADY_RUNNING TH_MAKE_LANG_STR(TH_LANG, _ERR_ALREADY_RUNNING)
#define TH_ERR_D3D_ERR_COULD_NOT_CREATE_OBJ TH_MAKE_LANG_STR(TH_LANG, _ERR_D3D_COULD_NOT_CREATE_OBJ)
#define TH_ERR_LOGGER_START TH_MAKE_LANG_STR(TH_LANG, _ERR_LOGGER_START)
#define TH_ERR_LOGGER_END TH_MAKE_LANG_STR(TH_LANG, _ERR_LOGGER_END)
#define TH_ERR_NO_PAD_FOUND TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_PAD_FOUND)
#define TH_ERR_OPTION_CHANGED_RESTART TH_MAKE_LANG_STR(TH_LANG, _ERR_OPTION_CHANGED_RESTART)
#define TH_ERR_SCREEN_INIT_32BITS TH_MAKE_LANG_STR(TH_LANG, _ERR_SCREEN_INIT_32BITS)
#define TH_ERR_SCREEN_INIT_16BITS TH_MAKE_LANG_STR(TH_LANG, _ERR_SCREEN_INIT_16BITS)
#define TH_ERR_SET_REFRESH_RATE_60HZ TH_MAKE_LANG_STR(TH_LANG, _ERR_SET_REFRESH_RATE_60HZ)
#define TH_ERR_TL_HAL_UNAVAILABLE TH_MAKE_LANG_STR(TH_LANG, _ERR_TL_HAL_UNAVAILABLE)
#define TH_ERR_HAL_UNAVAILABLE TH_MAKE_LANG_STR(TH_LANG, _ERR_HAL_UNAVAILABLE)
#define TH_ERR_D3D_INIT_FAILED TH_MAKE_LANG_STR(TH_LANG, _ERR_D3D_INIT_FAILED)
#define TH_ERR_BACKBUFFER_NONLOCKED TH_MAKE_LANG_STR(TH_LANG, _ERR_BACKBUFFER_NONLOCKED)
#define TH_ERR_CANT_CHANGE_REFRESH_RATE_FORCE_VSYNC TH_MAKE_LANG_STR(TH_LANG, _ERR_CANT_CHANGE_REFRESH_RATE_FORCE_VSYNC)
#define TH_USING_REF_MODE TH_MAKE_LANG_STR(TH_LANG, _USING_REF_MODE)
#define TH_USING_HAL_MODE TH_MAKE_LANG_STR(TH_LANG, _USING_HAL_MODE)
#define TH_USING_TL_HAL_MODE TH_MAKE_LANG_STR(TH_LANG, _USING_TL_HAL_MODE)
#define TH_ERR_NO_SUPPORT_FOR_D3DTEXOPCAPS_ADD TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_SUPPORT_FOR_D3DTEXOPCAPS_ADD)
#define TH_ERR_CANT_FORCE_60FPS_NO_ASYNC_FLIP TH_MAKE_LANG_STR(TH_LANG, _ERR_CANT_FORCE_60FPS_NO_ASYNC_FLIP)
#define TH_ERR_D3DFMT_A8R8G8B8_UNSUPPORTED TH_MAKE_LANG_STR(TH_LANG, _ERR_D3DFMT_A8R8G8B8_UNSUPPORTED)
#define TH_ERR_CONFIG_NOT_FOUND TH_MAKE_LANG_STR(TH_LANG, _ERR_CONFIG_NOT_FOUND)
#define TH_ERR_CONFIG_CORRUPTED TH_MAKE_LANG_STR(TH_LANG, _ERR_CONFIG_CORRUPTED)
#define TH_ERR_NO_WAVE_FILE TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_WAVE_FILE)
#define TH_ERR_NOT_A_WAV_FILE TH_MAKE_LANG_STR(TH_LANG, _ERR_NOT_A_WAV_FILE)
#define TH_ERR_NO_VERTEX_BUFFER TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_VERTEX_BUFFER)
#define TH_ERR_NO_FOG TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_FOG)
#define TH_ERR_USE_16BIT_TEXTURES TH_MAKE_LANG_STR(TH_LANG, _ERR_USE_16BIT_TEXTURES)
#define TH_ERR_FORCE_BACKBUFFER_CLEAR TH_MAKE_LANG_STR(TH_LANG, _ERR_FORCE_BACKBUFFER_CLEAR)
#define TH_ERR_DONT_RENDER_ITEMS TH_MAKE_LANG_STR(TH_LANG, _ERR_DONT_RENDER_ITEMS)
#define TH_ERR_NO_GOURAUD_SHADING TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_GOURAUD_SHADING)
#define TH_ERR_NO_DEPTH_TESTING TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_DEPTH_TESTING)
#define TH_ERR_FORCE_60FPS_MODE TH_MAKE_LANG_STR(TH_LANG, _ERR_FORCE_60FPS_MODE)
#define TH_ERR_NO_TEXTURE_COLOR_COMPOSITING TH_MAKE_LANG_STR(TH_LANG, _ERR_NO_TEXTURE_COLOR_COMPOSITING)
#define TH_ERR_LAUNCH_WINDOWED TH_MAKE_LANG_STR(TH_LANG, _ERR_LAUNCH_WINDOWED)
#define TH_ERR_FORCE_REFERENCE_RASTERIZER TH_MAKE_LANG_STR(TH_LANG, _ERR_FORCE_REFERENCE_RASTERIZER)
#define TH_ERR_DO_NOT_USE_DIRECTINPUT TH_MAKE_LANG_STR(TH_LANG, _ERR_DO_NOT_USE_DIRECTINPUT)
#define TH_ERR_DIRECTINPUT_NOT_AVAILABLE TH_MAKE_LANG_STR(TH_LANG, _ERR_DIRECTINPUT_NOT_AVAILABLE)
#define TH_ERR_DIRECTINPUT_SETDATAFORMAT_NOT_AVAILABLE TH_MAKE_LANG_STR(TH_LANG, _ERR_DIRECTINPUT_SETDATAFORMAT_NOT_AVAILABLE)
#define TH_ERR_DIRECTINPUT_SETCOOPERATIVELEVEL_NOT_AVAILABLE TH_MAKE_LANG_STR(TH_LANG, _ERR_DIRECTINPUT_SETCOOPERATIVELEVEL_NOT_AVAILABLE)
#define TH_ERR_DIRECTINPUT_INITIALIZED TH_MAKE_LANG_STR(TH_LANG, _ERR_DIRECTINPUT_INITIALIZED)
#define TH_ERR_PAD_FOUND TH_MAKE_LANG_STR(TH_LANG, _ERR_PAD_FOUND)
#define TH_ERR_FILE_CANNOT_BE_EXPORTED TH_MAKE_LANG_STR(TH_LANG, _ERR_FILE_CANNOT_BE_EXPORTED)
#define TH_ERR_FOLDER_HAS_WRITE_PROTECT_OR_DISK_FULL                                                                   \
    TH_MAKE_LANG_STR(TH_LANG, _ERR_FOLDER_HAS_WRITE_PROTECT_OR_DISK_FULL)
#define TH_ERR_ASCIIMANAGER_INIT_FAILED TH_MAKE_LANG_STR(TH_LANG, _ERR_ASCIIMANAGER_INIT_FAILED)
#define TH_ERR_WRONG_DATA_VERSION TH_MAKE_LANG_STR(TH_LANG, _ERR_WRONG_DATA_VERSION)
#define TH_ERR_CANNOT_BE_LOADED TH_MAKE_LANG_STR(TH_LANG, _ERR_CANNOT_BE_LOADED)
#define TH_ERR_ANMMANAGER_SPRITE_CORRUPTED TH_MAKE_LANG_STR(TH_LANG, _ERR_ANMMANAGER_SPRITE_CORRUPTED)
#define TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED TH_MAKE_LANG_STR(TH_LANG, _ERR_ANMMANAGER_TEXTURE_CORRUPTED)
#define TH_ERR_ANMMANAGER_UNK_TEX_FORMAT TH_MAKE_LANG_STR(TH_LANG, _ERR_ANMMANAGER_UNK_TEX_FORMAT)
#define TH_ERR_ECLMANAGER_ENEMY_DATA_CORRUPT TH_MAKE_LANG_STR(TH_LANG, _ERR_ECLMANAGER_ENEMY_DATA_CORRUPT)
#define TH_ERR_ENDING_END_FILE_CORRUPTED TH_MAKE_LANG_STR(TH_LANG, _ERR_ENDING_END_FILE_CORRUPTED)
#define TH_ERR_MAINMENU_LOAD_SELECT_SCREEN_FAILED TH_MAKE_LANG_STR(TH_LANG, _ERR_MAINMENU_LOAD_SELECT_SCREEN_FAILED)
#define TH_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER TH_MAKE_LANG_STR(TH_LANG, _ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER)
#define TH_ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT TH_MAKE_LANG_STR(TH_LANG, _ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT)
#define TH_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE TH_MAKE_LANG_STR(TH_LANG, _ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE)
#define TH_DBG_SOUNDPLAYER_INIT_SUCCESS TH_MAKE_LANG_STR(TH_LANG, _DBG_SOUNDPLAYER_INIT_SUCCESS)
#define TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_STAGE TH_MAKE_LANG_STR(TH_LANG, _ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_STAGE)
#define TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_PLAYER TH_MAKE_LANG_STR(TH_LANG, _ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_PLAYER)
#define TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_BULLETMANAGER TH_MAKE_LANG_STR(TH_LANG, _ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_BULLETMANAGER)
#define TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ENEMYMANAGER TH_MAKE_LANG_STR(TH_LANG, _ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ENEMYMANAGER)
#define TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ECLMANAGER TH_MAKE_LANG_STR(TH_LANG, _ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ECLMANAGER)
#define TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_EFFECTMANAGER TH_MAKE_LANG_STR(TH_LANG, _ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_EFFECTMANAGER)
#define TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_GUI TH_MAKE_LANG_STR(TH_LANG, _ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_GUI)
#define TH_ERR_GUI_MSG_FILE_CORRUPTED TH_MAKE_LANG_STR(TH_LANG, _ERR_GUI_MSG_FILE_CORRUPTED)
#define TH_ERR_STAGE_DATA_CORRUPTED TH_MAKE_LANG_STR(TH_LANG, _ERR_STAGE_DATA_CORRUPTED)
#define TH_ERR_MIDI_FAILED_TO_READ_FILE TH_MAKE_LANG_STR(TH_LANG, _ERR_MIDI_FAILED_TO_READ_FILE)
#define TH_ERR_MIDI_NOT_LOADED TH_MAKE_LANG_STR(TH_LANG, _ERR_MIDI_NOT_LOADED)
#define TH_REIMU_A_BOMB_NAME TH_MAKE_LANG_STR(TH_LANG, _REIMU_A_BOMB_NAME)
#define TH_REIMU_B_BOMB_NAME TH_MAKE_LANG_STR(TH_LANG, _REIMU_B_BOMB_NAME)
#define TH_MARISA_A_BOMB_NAME TH_MAKE_LANG_STR(TH_LANG, _MARISA_A_BOMB_NAME)
#define TH_MARISA_B_BOMB_NAME TH_MAKE_LANG_STR(TH_LANG, _MARISA_B_BOMB_NAME)
#define TH_HAKUREI_REIMU_SPIRIT TH_MAKE_LANG_STR(TH_LANG, _HAKUREI_REIMU_SPIRIT)
#define TH_HAKUREI_REIMU_DREAM TH_MAKE_LANG_STR(TH_LANG, _HAKUREI_REIMU_DREAM)
#define TH_KIRISAME_MARISA_DEVIL TH_MAKE_LANG_STR(TH_LANG, _KIRISAME_MARISA_DEVIL)
#define TH_KIRISAME_MARISA_LOVE TH_MAKE_LANG_STR(TH_LANG, _KIRISAME_MARISA_LOVE)
#define TH_SATSUKI_RIN_FLOWER TH_MAKE_LANG_STR(TH_LANG, _SATSUKI_RIN_FLOWER)
#define TH_SATSUKI_RIN_WIND TH_MAKE_LANG_STR(TH_LANG, _SATSUKI_RIN_WIND)
#define TH_UNKNOWN_SPELLCARD TH_MAKE_LANG_STR(TH_LANG, _UNKNOWN_SPELLCARD)
#define TH_CM_DAT_FILE TH_MAKE_LANG_STR(TH_LANG, _CM_DAT_FILE)
#define TH_ED_DAT_FILE TH_MAKE_LANG_STR(TH_LANG, _ED_DAT_FILE)
#define TH_IN_DAT_FILE TH_MAKE_LANG_STR(TH_LANG, _IN_DAT_FILE)
#define TH_MD_DAT_FILE TH_MAKE_LANG_STR(TH_LANG, _MD_DAT_FILE)
#define TH_ST_DAT_FILE TH_MAKE_LANG_STR(TH_LANG, _ST_DAT_FILE)
#define TH_TL_DAT_FILE TH_MAKE_LANG_STR(TH_LANG, _TL_DAT_FILE)

#define TH_SONG_NAME "♪%s"
#define TH_FONT_NAME "ＭＳ ゴシック"
