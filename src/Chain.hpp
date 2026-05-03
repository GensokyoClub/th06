#pragma once

#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

#include <windows.h>

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
    short priority;
    u16 isHeapAllocated : 1;
    ChainCallback callback;
    ChainAddedCallback addedCallback;
    ChainDeletedCallback deletedCallback;
    struct ChainElem *prev;
    struct ChainElem *next;
    struct ChainElem *unkPtr;
    void *arg;

    ChainElem();
    ~ChainElem();
};
ZUN_ASSERT_SIZE(ChainElem, 0x20);

class Chain
{
  private:
    ChainElem calcChain;
    ChainElem drawChain;
    unsigned int midiOutputDeviceCount;
    unsigned int unk;

    void ReleaseSingleChain(ChainElem *root);

  public:
    Chain();
    ~Chain();

    void Cut(ChainElem *to_remove);
    void Release(void);
    int AddToCalcChain(ChainElem *elem, int priority);
    int AddToDrawChain(ChainElem *elem, int priority);
    int RunDrawChain(void);
    int RunCalcChain(void);

    ChainElem *CreateElem(ChainCallback callback);
};
ZUN_ASSERT_SIZE(Chain, 0x48);

DIFFABLE_EXTERN(Chain, g_Chain)
}; // namespace th06
