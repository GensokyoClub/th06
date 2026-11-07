#include "stdafx.h"

#include "MusicRoom.hpp"

#include "AsciiManager.hpp"
#include "ChainPriorities.hpp"

namespace th06
{

ZunResult MusicRoom::CheckInputEnable()
{
    if (this->waitFramesCount >= 8)
    {
        this->enableInput = true;
    }

    return ZUN_SUCCESS;
}

ZunBool MusicRoom::ProcessInput()
{
    // This variable is never used after this?
    i32 listPos = this->listingOffset;

    if (WAS_PRESSED(TH_BUTTON_UP))
    {
        // Vertical wrap-around
        if (--this->cursor < 0)
        {
#if !TRIALBUILD
            this->cursor = this->numDescriptors - 1;
            this->listingOffset = this->numDescriptors - 10;
#else
            this->cursor = 6;
            this->listingOffset = 0;
#endif
        }
        // Scroll list up
        else if (this->listingOffset > this->cursor)
        {
            this->listingOffset = this->cursor;
        }
    }

    if (WAS_PRESSED(TH_BUTTON_DOWN))
    {
        // Vertical wrap-around
#if !TRIALBUILD
        if (++this->cursor >= this->numDescriptors)
#else
        if (++this->cursor >= 7)
#endif
        {
            this->cursor = 0;
            this->listingOffset = 0;
        }
        else
        {
            // Scroll list down
            if (this->listingOffset <= this->cursor - 10)
            {
                this->listingOffset = this->cursor - 9;
            }
        }
    }

    if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
    {
        this->selectedSongIndex = this->cursor;
        g_Supervisor.PlayAudio(this->trackDescriptors[this->selectedSongIndex].path);

        // Update description to match newly selected song
        for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->descriptionSprites); i++)
        {
            char lineCharBuffer[64];
            memset(lineCharBuffer, 0, sizeof(lineCharBuffer));

            if (i % 2 == 0 || strlen(this->trackDescriptors[this->selectedSongIndex].description[i / 2]) > 32)
            {
                memcpy(lineCharBuffer,
                       &this->trackDescriptors[this->selectedSongIndex].description[i / 2][(i % 2) * 32], 32);
            }

            if (lineCharBuffer[0] != '\0')
            {
                this->descriptionSprites[i].flags.isVisibleOverride = true;
                g_AnmManager->DrawVmTextFmt(&this->descriptionSprites[i], COLOR_MUSIC_ROOM_SONG_DESC_TEXT,
                                            COLOR_MUSIC_ROOM_SONG_DESC_SHADOW, lineCharBuffer);
            }
            else
            {
                this->descriptionSprites[i].flags.isVisibleOverride = false;
            }

            this->descriptionSprites[i].pos.x = ((f32)(i % 2)) * 248.0f + 96.0f;
            this->descriptionSprites[i].pos.y = 320.0f + ((i / 2) << 4);
            this->descriptionSprites[i].pos.z = 0.0f;
            this->descriptionSprites[i].flags.anchor = AnmVmAnchor_TopLeft;
        }
    }

    if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
    {
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        return true;
    }

    return false;
}

static ChainCallbackResult MusicRoom_OnUpdate(MusicRoom *musicRoom)
{
    ZunBool oldInputSetting = musicRoom->enableInput;
    for (;;)
    {
        switch (musicRoom->enableInput)
        {
        case false:
            if (!musicRoom->CheckInputEnable())
            {
                break;
            }

            continue;

        case true:
            if (musicRoom->ProcessInput())
            {
                return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
            }
        }
        break;
    }

    if (oldInputSetting != musicRoom->enableInput)
    {
        musicRoom->waitFramesCount = 0;
    }
    else
    {
        musicRoom->waitFramesCount++;
    }
    g_AnmManager->ExecuteScript(musicRoom->mainVm);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult MusicRoom_OnDraw(MusicRoom *musicRoom)
{
    i32 i;
    D3DXVECTOR3 textPos;
    char rightArrowStr[2];

    rightArrowStr[0] = TEXT_RIGHT_ARROW;
    rightArrowStr[1] = '\0';

    g_AnmManager->ClearTexture();
    g_AnmManager->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    g_AnmManager->DrawNoRotation(musicRoom->mainVm);

    // Draw the 10 songs in the song select window, and list indices
    for (i = musicRoom->listingOffset; i < musicRoom->listingOffset + 10; i++)
    {
        if (musicRoom->cursor != i)
        {
            musicRoom->titleSprites[i].color = COLOR_SET_ALPHA(COLOR_GREY, 0xe0);
            g_AsciiManager.SetColor(COLOR_SET_ALPHA(COLOR_GREY, 0xe0));
        }
        else
        {
            musicRoom->titleSprites[i].color = COLOR_WHITE;
            g_AsciiManager.SetColor(COLOR_WHITE);
        }

        musicRoom->titleSprites[i].pos.x = 93.0f;
        musicRoom->titleSprites[i].pos.y = 104.0f + (((i + 1) - musicRoom->listingOffset) * 18) - 20.0f;
        musicRoom->titleSprites[i].pos.z = 0.0f;
        g_AnmManager->DrawNoRotation(&musicRoom->titleSprites[i]);

        textPos = musicRoom->titleSprites[i].pos;
        textPos.x -= 60.0f;

        if (musicRoom->cursor == i)
        {
            g_AsciiManager.AddString(&textPos, rightArrowStr);
        }

        textPos.x += 15.0f;
        g_AsciiManager.AddFormatText(&textPos, "%2d.", i + 1);
    }

    i++; // ???

    for (i = 0; i < ARRAY_SIZE_SIGNED(musicRoom->descriptionSprites); i++)
    {
        g_AnmManager->DrawNoRotation(&musicRoom->descriptionSprites[i]);
    }

    g_AsciiManager.SetColor(COLOR_WHITE);

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(i, lineIndex, currChar, charIndex, fileBase)
static ZunResult MusicRoom_AddedCallback(MusicRoom *musicRoom)
{
    u32 charIndex;
    char *currChar;
    char *fileBase;
    i32 i;
    i32 lineIndex;

    if (g_AnmManager->LoadSurface(0, "data/result/music.jpg") != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    if (g_AnmManager->LoadAnm(ANM_FILE_MUSIC00, "data/music00.anm", ANM_OFFSET_MUSIC00) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    if (g_AnmManager->LoadAnm(ANM_FILE_MUSIC01, "data/music01.anm", ANM_OFFSET_MUSIC01) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    if (g_AnmManager->LoadAnm(ANM_FILE_MUSIC02, "data/music02.anm", ANM_OFFSET_MUSIC02) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_AnmManager->SetAndExecuteScriptIdx(musicRoom->mainVm, ANM_OFFSET_MUSIC00);
    musicRoom->waitFramesCount = 0;
    currChar = (char *)FileSystem::OpenPath("data/musiccmt.txt");
    fileBase = currChar;

    if (currChar == NULL)
    {
        return ZUN_ERROR;
    }

    musicRoom->trackDescriptors = ZUN_NEW_ARRAY(TrackDescriptor, ARRAY_SIZE_SIGNED(musicRoom->titleSprites));

    i = -1;
    while (currChar - fileBase < g_ReadFileSize)
    {
        if (*currChar == '@')
        {
            currChar++;
            i++;
            charIndex = 0;

            while (*currChar != '\n' && *currChar != '\r')
            {
                musicRoom->trackDescriptors[i].path[charIndex] = *currChar;
                currChar++;
                charIndex++;
                if (currChar - fileBase >= g_ReadFileSize)
                {
                    goto finishMusiccmtRead;
                }
            }

            while (*currChar == '\n' || *currChar == '\r')
            {
                currChar++;
                if (currChar - fileBase >= g_ReadFileSize)
                {
                    goto finishMusiccmtRead;
                }
            }

            charIndex = 0;
            while (*currChar != '\n' && *currChar != '\r')
            {
                musicRoom->trackDescriptors[i].title[charIndex] = *currChar;
                currChar++;
                charIndex++;
                if (currChar - fileBase >= g_ReadFileSize)
                {
                    goto finishMusiccmtRead;
                }
            }

            // Dead code. Is it a bug? Was it an intentional quick and dirty change? Who knows?
            // Has the effect of offsetting the description text by a line
            while (*currChar == '\n' && *currChar == '\r')
            {
                currChar++;
                if (currChar - fileBase >= g_ReadFileSize)
                {
                    goto finishMusiccmtRead;
                }
            }

            for (lineIndex = 0; lineIndex < 8; lineIndex++)
            {
                if (*currChar == '@')
                {
                    break;
                }

                memset(musicRoom->trackDescriptors[i].description[lineIndex], 0,
                       sizeof(musicRoom->trackDescriptors[i].description[lineIndex]));
                charIndex = 0;
                while (*currChar != '\n' && *currChar != '\r')
                {
                    musicRoom->trackDescriptors[i].description[lineIndex][charIndex] = *currChar;
                    currChar++;
                    charIndex++;
                    if (currChar - fileBase >= g_ReadFileSize)
                    {
                        goto finishMusiccmtRead;
                    }
                }

                while (*currChar == '\n' || *currChar == '\r')
                {
                    currChar++;
                    if (currChar - fileBase >= g_ReadFileSize)
                    {
                        goto finishMusiccmtRead;
                    }
                }
            }
        }
        else
        {
            currChar++;
        }
    }

finishMusiccmtRead:
    musicRoom->numDescriptors = i + 1;

    for (i = 0; i < musicRoom->numDescriptors; i++)
    {
        g_AnmManager->InitializeAndSetSprite(&musicRoom->titleSprites[i], ANM_OFFSET_MUSIC01 + i);
        g_AnmManager->DrawVmTextFmt(&musicRoom->titleSprites[i], COLOR_MUSIC_ROOM_SONG_TITLE_TEXT,
                                    COLOR_MUSIC_ROOM_SONG_TITLE_SHADOW, musicRoom->trackDescriptors[i].title);
        musicRoom->titleSprites[i].pos.x = 93.0f;
        musicRoom->titleSprites[i].pos.y = 104.0f + ((i + 1) * 18) - 20.0f;
        musicRoom->titleSprites[i].pos.z = 0.0f;
        musicRoom->titleSprites[i].flags.anchor = AnmVmAnchor_TopLeft;
    }

    // Two sprites are used for each line, with a split at the 32nd byte
    for (i = 0; i < ARRAY_SIZE_SIGNED(musicRoom->descriptionSprites); i++)
    {
        g_AnmManager->InitializeAndSetSprite(&musicRoom->descriptionSprites[i], ANM_SCRIPT_TEXT_MUSIC_ROOM_DESC + i);

        char lineCharBuffer[64];
        memset(lineCharBuffer, 0, sizeof(lineCharBuffer));

        if (i % 2 == 0 || strlen(musicRoom->trackDescriptors[musicRoom->selectedSongIndex].description[i / 2]) > 32)
        {
            memcpy(lineCharBuffer, &musicRoom->trackDescriptors[0].description[i / 2][(i % 2) * 32], 32);
        }

        if (lineCharBuffer[0] != '\0')
        {
            musicRoom->descriptionSprites[i].flags.isVisibleOverride = true;
            g_AnmManager->DrawVmTextFmt(&musicRoom->descriptionSprites[i], COLOR_MUSIC_ROOM_SONG_DESC_TEXT,
                                        COLOR_MUSIC_ROOM_SONG_DESC_SHADOW, lineCharBuffer);
        }
        else
        {
            musicRoom->descriptionSprites[i].flags.isVisibleOverride = false;
        }

        musicRoom->descriptionSprites[i].pos.x = ((f32)(i % 2)) * 248.0f + 96.0f;
        musicRoom->descriptionSprites[i].pos.y = 320.0f + ((i / 2) << 4);
        musicRoom->descriptionSprites[i].pos.z = 0.0f;
        musicRoom->descriptionSprites[i].flags.anchor = AnmVmAnchor_TopLeft;
    }

    ZUN_FREE(fileBase);

    return ZUN_SUCCESS;
}

static ZunResult MusicRoom_DeletedCallback(MusicRoom *musicRoom)
{
    ZUN_DELETE(musicRoom->trackDescriptors);

    g_AnmManager->ReleaseSurface(0);
    g_AnmManager->ReleaseAnm(ANM_FILE_MUSIC00);
    g_AnmManager->ReleaseAnm(ANM_FILE_MUSIC01);
    g_AnmManager->ReleaseAnm(ANM_FILE_MUSIC02);
    g_FuncChain.Cut(musicRoom->draw_chain);
    musicRoom->draw_chain = NULL;

    return ZUN_SUCCESS;
}

ZunResult MusicRoom_RegisterChain()
{
    static MusicRoom g_MusicRoom;
    MusicRoom *musicRoom = &g_MusicRoom;
    memset(musicRoom, 0, sizeof(MusicRoom));

    musicRoom->calc_chain = g_FuncChain.CreateElem((ChainCallback)MusicRoom_OnUpdate);
    musicRoom->calc_chain->arg = musicRoom;
    musicRoom->calc_chain->addedCallback = (ChainAddedCallback)MusicRoom_AddedCallback;
    musicRoom->calc_chain->deletedCallback = (ChainDeletedCallback)MusicRoom_DeletedCallback;

    if (g_FuncChain.AddToCalcChain(musicRoom->calc_chain, TH_CHAIN_PRIO_CALC_MAINMENU) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    musicRoom->draw_chain = g_FuncChain.CreateElem((ChainCallback)MusicRoom_OnDraw);
    musicRoom->draw_chain->arg = musicRoom;
    g_FuncChain.AddToDrawChain(musicRoom->draw_chain, TH_CHAIN_PRIO_DRAW_MAINMENU);

    return ZUN_SUCCESS;
}

} // namespace th06
