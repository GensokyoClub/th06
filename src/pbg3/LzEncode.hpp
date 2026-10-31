#pragma once
#include "decomp.hpp"
#include "pbg3/IPbg3Reader.hpp"
#include "pbg3/LzssTree.hpp"
#include "pbg3/Pbg3Writer.hpp"

namespace th06
{
struct LzEncodeEntry
{
    FILETIME lastWriteTime;
    u32 size;
    u32 offset;
    u32 checksum;
    char filename[256];
};
ZUN_ASSERT_SIZE(LzEncodeEntry, 0x114);

class LzEncodeQueue
{
    struct Node
    {
        LzEncodeEntry entry;
        Node *next;
    };
    typedef ZUN_ASSERT_SIZE(Node, 0x118) NodeSizeCheck;

  public:
    LzEncodeQueue() : first(NULL), last(NULL), count(0)
    {
    }
    ~LzEncodeQueue()
    {
        if (this->count != 0)
        {
            while (this->first != NULL)
            {
                // NOTE: This specific read pattern convinces
                // MSVC that it still needs to NULL check old.
                // Nice anti-pattern PBG.
                Node *old = this->first;
                this->first = this->first->next;
                SAFE_DELETE(old);
            }
        }
    }
    void Push(const LzEncodeEntry &entry)
    {
        Node *node = new Node;
        if (node != NULL)
        {
            node->entry = entry;
            if (this->last != NULL)
            {
                this->last->next = node;
            }
            else
            {
                this->first = node;
            }
            this->last = node;
            node->next = NULL;
            ++this->count;
        }
    }
    void Pop(LzEncodeEntry &entry)
    {
        if (this->count == 0)
        {
            return;
        }
        entry = this->first->entry;
        Node *old = this->first;
        this->first = this->first->next;
        if (this->first == NULL)
        {
            this->last = NULL;
        }
        SAFE_DELETE(old);
        --this->count;
    }

  private:
    Node *first;
    Node *last;
    u32 count;
};
ZUN_ASSERT_SIZE(LzEncodeQueue, 0xc);

class LzEncode : protected LzssTree
{
  public:
    BOOL Open(const char *path);
    BOOL Close();
    BOOL Encode(IPbg3Reader *input, const char *name);
    LzEncode();
    ~LzEncode();

  private:
    Pbg3Writer *writer;
    u32 entryCount;
    LzEncodeQueue *entries;
};
ZUN_ASSERT_SIZE(LzEncode, 0x1a018);
} // namespace th06
