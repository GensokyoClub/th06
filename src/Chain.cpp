#include "Chain.hpp"
#include "utils.hpp"

#include <new>

namespace th06
{
Chain g_Chain;

Chain::~Chain()
{
}

ChainElem::ChainElem()
{
    prev = NULL;
    next = NULL;
    callback = NULL;
    unkPtr = this;
    addedCallback = NULL;
    deletedCallback = NULL;
    priority = 0;

    isHeapAllocated = false;
}

ChainElem::~ChainElem()
{
    if (deletedCallback != NULL)
    {
        this->deletedCallback(this->arg);
    }

    prev = NULL;
    next = NULL;
    callback = NULL;
    addedCallback = NULL;
    deletedCallback = NULL;
}

Chain::Chain()
{
}

int Chain::AddToCalcChain(ChainElem *elem, int priority)
{
    ChainElem *cur;

    cur = &this->calcChain;
    utils::DebugPrint2("add calc chain (pri = %d)\n", priority);
    elem->priority = priority;

    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }

        cur = cur->next;
    }

    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;

        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }

        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }

    if (elem->addedCallback != NULL)
    {
        int res = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;

        return res;
    }
    else
    {
        return 0;
    }
}

int Chain::AddToDrawChain(ChainElem *elem, int priority)
{
    ChainElem *cur;

    cur = &this->drawChain;
    utils::DebugPrint2("add draw chain (pri = %d)\n", priority);
    elem->priority = priority;

    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }

        cur = cur->next;
    }

    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;

        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }

        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }

    if (elem->addedCallback != NULL)
    {
        return elem->addedCallback(elem->arg);
    }
    else
    {
        return 0;
    }
}

int Chain::RunCalcChain(void)
{
    ChainElem *tmp1;
    ChainElem *current;
    int updatedCount;

restart_from_first_job:
    updatedCount = 0;
    current = &this->calcChain;

    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        execute_again:
            switch (current->callback(current->arg))
            {
            case CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB:
                tmp1 = current;
                current = current->next;
                Cut(tmp1);

                updatedCount++;
                continue;

            case CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN:
                goto execute_again;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS:
                return 0;

            case CHAIN_CALLBACK_RESULT_BREAK:
                return 1;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR:
                return -1;

            case CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB:
                goto restart_from_first_job;

            default:
                break;
            }

            updatedCount++;
        }

        current = current->next;
    }

    return updatedCount;
}

int Chain::RunDrawChain(void)
{
    ChainElem *tmp1;
    ChainElem *current;
    int updatedCount;

    updatedCount = 0;
    current = &this->drawChain;

    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        execute_again:
            switch (current->callback(current->arg))
            {
            case CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB:
                tmp1 = current;
                current = current->next;
                Cut(tmp1);

                updatedCount++;
                continue;

            case CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN:
                goto execute_again;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS:
                return 0;

            case CHAIN_CALLBACK_RESULT_BREAK:
                return 1;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR:
                return -1;

            default:
                break;
            }

            updatedCount++;
        }

        current = current->next;
    }

    return updatedCount;
}

void Chain::ReleaseSingleChain(ChainElem *root)
{
    // NOTE: Those names are like this to get perfect stack frame matching
    // TODO: Give meaningfull names that still match.
    ChainElem a0;
    ChainElem *current;
    ChainElem *tmp;
    ChainElem *wasNext;

    tmp = new ChainElem();
    a0.next = tmp;

    current = root;
    while (current != NULL)
    {
        tmp->unkPtr = current;
        tmp->next = new ChainElem();
        tmp = tmp->next;
        current = current->next;
    }

    current = &a0;
    while (current != NULL)
    {
        Cut(current->unkPtr);
        current = current->next;
    }

    tmp = a0.next;

    while (tmp != NULL)
    {
        wasNext = tmp->next;

        delete tmp;

        tmp = NULL;
        tmp = wasNext;
    }
}

void Chain::Release(void)
{
    ReleaseSingleChain(&this->calcChain);
    ReleaseSingleChain(&this->drawChain);
}

ChainElem *Chain::CreateElem(ChainCallback callback)
{
    ChainElem *elem;

    elem = new ChainElem();

    elem->callback = callback;
    elem->addedCallback = NULL;
    elem->deletedCallback = NULL;

    elem->isHeapAllocated = true;

    return elem;
}

void Chain::Cut(ChainElem *to_remove)
{
    int isDrawChain;
    ChainElem *tmp;

    isDrawChain = 0;

    if (to_remove == NULL)
    {
        return;
    }

    tmp = &this->calcChain;

    while (tmp != NULL)
    {
        if (tmp == to_remove)
        {
            goto destroy_elem;
        }

        tmp = tmp->next;
    }

    {
        isDrawChain = 1;

        tmp = &this->drawChain;
        while (tmp != NULL)
        {
            if (tmp == to_remove)
            {
                goto destroy_elem;
            }

            tmp = tmp->next;
        }
    }

    return;

destroy_elem:
    if (!isDrawChain)
    {
        utils::DebugPrint2("calc cut Chain (Pri = %d)\n", to_remove->priority);
    }
    else
    {
        utils::DebugPrint2("draw cut Chain (Pri = %d)\n", to_remove->priority);
    }

    if (to_remove->prev != NULL)
    {
        to_remove->callback = NULL;
        to_remove->prev->next = to_remove->next;

        if (to_remove->next != NULL)
        {
            to_remove->next->prev = to_remove->prev;
        }

        to_remove->prev = NULL;
        to_remove->next = NULL;

        if (to_remove->isHeapAllocated)
        {
            delete to_remove;
            to_remove = NULL;
        }
        else
        {
            if (to_remove->deletedCallback != NULL)
            {
                ChainDeletedCallback callback = to_remove->deletedCallback;
                to_remove->deletedCallback = NULL;
                callback(to_remove->arg);
            }
        }
    }
}
}; // namespace th06
