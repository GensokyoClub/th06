#include "pbg3/LzEncode.hpp"
#include <string.h>

namespace th06
{
void LzssTree::SetRoot(i32 position)
{
    this->nodes[LZSS_DICTSIZE].right = position;
    this->nodes[position].parent = LZSS_DICTSIZE;
    this->nodes[position].right = 0;
    this->nodes[position].left = 0;
}

void LzssTree::Reset()
{
    memset(this->window, 0, sizeof(this->window));
    for (i32 i = 0; i <= LZSS_DICTSIZE; ++i)
    {
        this->nodes[i].parent = 0;
        this->nodes[i].left = 0;
        this->nodes[i].right = 0;
    }
}

void LzssTree::Contract(i32 oldPosition, i32 newPosition)
{
    this->nodes[newPosition].parent = this->nodes[oldPosition].parent;
    if (this->nodes[this->nodes[oldPosition].parent].right == oldPosition)
    {
        this->nodes[this->nodes[oldPosition].parent].right = newPosition;
    }
    else
    {
        this->nodes[this->nodes[oldPosition].parent].left = newPosition;
    }
    this->nodes[oldPosition].parent = 0;
}

void LzssTree::Replace(i32 oldPosition, i32 newPosition)
{
    i32 parent = this->nodes[oldPosition].parent;
    if (this->nodes[parent].left == oldPosition)
    {
        this->nodes[parent].left = newPosition;
    }
    else
    {
        this->nodes[parent].right = newPosition;
    }
    this->nodes[newPosition] = this->nodes[oldPosition];
    this->nodes[this->nodes[newPosition].left].parent = newPosition;
    this->nodes[this->nodes[newPosition].right].parent = newPosition;
    this->nodes[oldPosition].parent = 0;
}

i32 LzssTree::FindPredecessor(i32 position)
{
    i32 next;

    next = this->nodes[position].left;
    while (this->nodes[next].right != 0)
    {
        next = this->nodes[next].right;
    }
    return next;
}

LzssTree::LzssTree()
{
    this->Reset();
}

i32 LzssTree::Insert(i32 position, i32 *matchPosition)
{
    i32 length;
    i32 candidate;
    i32 difference;
    i32 best;
    i32 *next;

    if (position == 0)
    {
        return 0;
    }
    candidate = this->nodes[LZSS_DICTSIZE].right;
    best = 0;
    for (;;)
    {
        for (length = 0; length < LZSS_LOOKAHEAD_MAX; length++)
        {
            difference =
                this->window[LZSS_DICTPOS_MOD(position, length)] - this->window[LZSS_DICTPOS_MOD(candidate, length)];
            if (difference != 0)
            {
                break;
            }
        }
        if (length >= best)
        {
            best = length;
            *matchPosition = candidate;
            if (best >= LZSS_LOOKAHEAD_MAX)
            {
                this->Replace(candidate, position);
                return best;
            }
        }
        if (difference >= 0)
        {
            next = &this->nodes[candidate].right;
        }
        else
        {
            next = &this->nodes[candidate].left;
        }
        if (*next == 0)
        {
            *next = position;
            this->nodes[position].parent = candidate;
            this->nodes[position].right = 0;
            this->nodes[position].left = 0;
            return best;
        }
        candidate = *next;
    }
}

void LzssTree::Delete(i32 position)
{
    i32 replacement;

    if (this->nodes[position].parent == 0)
    {
        return;
    }
    if (this->nodes[position].right == 0)
    {
        this->Contract(position, this->nodes[position].left);
    }
    else if (this->nodes[position].left == 0)
    {
        this->Contract(position, this->nodes[position].right);
    }
    else
    {
        replacement = this->FindPredecessor(position);
        this->Delete(replacement);
        this->Replace(position, replacement);
    }
}

LzEncode::LzEncode()
{
    this->entryCount = 0;
    this->entries = NULL;
    this->writer = NULL;
    this->Reset();
}

BOOL LzEncode::Close()
{
    if (this->writer == NULL)
    {
        return TRUE;
    }
    if (this->entries == NULL)
    {
        return FALSE;
    }
    this->writer->SeekToNextByte();
    u32 tableOffset = this->writer->Tell();
    LzEncodeEntry entry;
    for (u32 i = 0; i < this->entryCount; ++i)
    {
        this->entries->Pop(entry);
        this->writer->WriteVarInt(entry.lastWriteTime.dwHighDateTime);
        this->writer->WriteVarInt(entry.lastWriteTime.dwLowDateTime);
        this->writer->WriteVarInt(entry.checksum);
        this->writer->WriteVarInt(entry.offset);
        this->writer->WriteVarInt(entry.size);
        this->writer->IPbg3Writer::WriteString(entry.filename);
    }
    this->writer->SeekToOffset(4);
    this->writer->WriteVarInt(this->entryCount);
    this->writer->WriteVarInt(tableOffset);
    this->writer->SeekToNextByte();
    this->writer->Close();
    SAFE_DELETE(this->writer);
    SAFE_DELETE(this->entries);
    this->entryCount = 0;
    return TRUE;
}

BOOL LzEncode::Encode(IPbg3Reader *input, const char *name)
{
    if (this->writer == NULL)
    {
        return FALSE;
    }
    if (input == NULL)
    {
        return FALSE;
    }
    if (name == NULL)
    {
        return FALSE;
    }
    LzEncodeEntry entry;
    entry.offset = this->writer->Tell();
    if (input->SeekToOffset(0) == FALSE)
    {
        return FALSE;
    }
    if (this->writer->SeekToNextByte() == FALSE)
    {
        return FALSE;
    }
    this->writer->ResetChecksum();
    this->Reset();
    i32 i;
    i32 c;
    i32 available;
    i32 head;
    i32 consumed;
    i32 matchLength;
    i32 matchPosition;

    head = 1;
    for (i = 0; i < LZSS_LOOKAHEAD_MAX; i++)
    {
        if ((c = input->ReadByte()) == PBG_EOF)
        {
            break;
        }
        this->window[head + i] = (u8)c;
    }
    available = i;
    this->SetRoot(head);
    matchLength = 0;
    matchPosition = 0;
    while (available > 0)
    {
        if (matchLength > available)
        {
            matchLength = available;
        }
        if (matchLength <= LZSS_BREAKEVEN - 1)
        {
            consumed = 1;
            this->writer->WriteBit(1);
            this->writer->WriteBits(this->window[head], 8);
        }
        else
        {
            this->writer->WriteBit(0);
            this->writer->WriteBits(matchPosition, LZSS_OFFSET_BITS);
            this->writer->WriteBits(matchLength - LZSS_BREAKEVEN, LZSS_LENGTH_BITS);
            consumed = matchLength;
        }
        for (i = 0; i < consumed; i++)
        {
            this->Delete(LZSS_DICTPOS_MOD(head, LZSS_LOOKAHEAD_MAX));
            if ((c = input->ReadByte()) == PBG_EOF)
            {
                available--;
            }
            else
            {
                this->window[LZSS_DICTPOS_MOD(head, LZSS_LOOKAHEAD_MAX)] = (u8)c;
            }
            head = LZSS_DICTPOS_MOD(head, 1);
            if (available)
            {
                matchLength = this->Insert(head, &matchPosition);
            }
        }
    }
    this->writer->WriteBit(0);
    this->writer->WriteBits(0, LZSS_OFFSET_BITS);
    if (this->writer->SeekToNextByte() == FALSE)
    {
        return FALSE;
    }
    entry.checksum = this->writer->GetChecksum();
    entry.size = input->GetSize();
    input->GetLastWriteTime(&entry.lastWriteTime);

    // NOTE: Very different strcpy codegen
    strcpy(entry.filename, name);
    this->entries->Push(entry);
    input->SeekToOffset(0);
    ++this->entryCount;
    return TRUE;
}

LzEncode::~LzEncode()
{
    this->Close();
}

BOOL LzEncode::Open(const char *path)
{
    if (this->Close() == FALSE)
    {
        return FALSE;
    }
    this->writer = new Pbg3Writer;
    if (this->writer == NULL)
    {
        return FALSE;
    }
    if (this->writer->OpenArchive(path) == FALSE)
    {
        SAFE_DELETE(this->writer);
        return FALSE;
    }
    this->entryCount = 0;
    this->entries = new LzEncodeQueue;
    if (this->entries == NULL)
    {
        SAFE_DELETE(this->writer);
        return FALSE;
    }
    this->writer->WriteInt(PBG3_MAGIC);
    this->writer->WriteInt(0);
    this->writer->WriteInt(0);
    this->writer->WriteChar(0);
    return TRUE;
}
} // namespace th06
