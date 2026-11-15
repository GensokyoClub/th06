#pragma once
#include "ZunResult.hpp"
#include "decomp.hpp"

#include <Windows.h>

namespace th06
{

enum ChainCallbackResult
{
    CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB = (unsigned int)0,
    CHAIN_CALLBACK_RESULT_CONTINUE = (unsigned int)1,
    CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN = (unsigned int)2,
    CHAIN_CALLBACK_RESULT_BREAK = (unsigned int)3,
    CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS = (unsigned int)4,
    CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR = (unsigned int)5,
    CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB = (unsigned int)6,
};

// TODO
typedef ChainCallbackResult (*ChainCallback)(void *);
typedef ZunResult (*ChainAddedCallback)(void *);
typedef ZunResult (*ChainDeletedCallback)(void *);

class ChainElem
{
  public:
    ChainElem();
    ~ChainElem();

    void SetCallback(ChainCallback callback)
    {
        this->callback = callback;
        this->addedCallback = NULL;
        this->deletedCallback = NULL;
    }

    i16 priority;
    u16 isHeapAllocated : 1;
    alignment_bitfields(u16, 15);
    ChainCallback callback;
    ChainAddedCallback addedCallback;
    ChainDeletedCallback deletedCallback;
    ChainElem *prev;
    ChainElem *next;
    ChainElem *unkPtr;
    void *arg;
};
ZUN_ASSERT_TYPE(ChainElem, 0x20, 4);

class Chain
{
  private:
    ChainElem calcChain;
    ChainElem drawChain;
    u32 midiOutputDeviceCount;
    u32 unk;
    // Only the first 0x48 bytes are referenced.
    unreferenced_fields(0x34);

    void ReleaseSingleChain(ChainElem *root);

  public:
    Chain();
    ~Chain();

    void Cut(ChainElem *to_remove);
    void Release();
    ZunResult AddToCalcChain(ChainElem *elem, i32 priority);
    ZunResult AddToDrawChain(ChainElem *elem, i32 priority);
    int RunDrawChain();
    int RunCalcChain();

    ChainElem *CreateElem(ChainCallback callback);
};
ZUN_ASSERT_TYPE(Chain, 0x7c, 4);

extern Chain g_FuncChain;
} // namespace th06
