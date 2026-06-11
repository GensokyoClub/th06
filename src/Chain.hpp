#pragma once

#include "ZunResult.hpp"
#include "inttypes.hpp"

enum ChainCallbackResult : unsigned int {
    CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB = 0,
    CHAIN_CALLBACK_RESULT_CONTINUE = 1,
    CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN = 2,
    CHAIN_CALLBACK_RESULT_BREAK = 3,
    CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS = 4,
    CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR = 5,
    CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB = 6,
};

// TODO
typedef ChainCallbackResult (*ChainCallback)(void *);
typedef ZunResult (*ChainAddedCallback)(void *);
typedef ZunResult (*ChainDeletedCallback)(void *);

class ChainElem {
  public:
    short priority;
    u16 isHeapAllocated : 1;
    ChainCallback callback;
    ChainAddedCallback addedCallback;
    ChainDeletedCallback deletedCallback;
    ChainElem *prev;
    ChainElem *next;
    ChainElem *unkPtr;
    void *arg;

    ChainElem();
    ~ChainElem();
};

class Chain {
  private:
    ChainElem calcChain;
    ChainElem drawChain;

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

extern Chain g_Chain;
