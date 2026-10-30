#pragma once
#include "decomp.hpp"

#define LZSS_OFFSET_BITS 13
#define LZSS_LENGTH_BITS 4
#define LZSS_DICTSIZE (1 << LZSS_OFFSET_BITS)
#define LZSS_BREAKEVEN 3
#define LZSS_LOOKAHEAD_MAX ((1 << LZSS_LENGTH_BITS) + LZSS_BREAKEVEN - 1)
#define LZSS_DICTSIZE_MASK (LZSS_DICTSIZE - 1)
#define LZSS_DICTPOS_MOD(pos, amount) ((pos + amount) & LZSS_DICTSIZE_MASK)

namespace th06
{
class LzssTree
{
  public:
    void SetRoot(i32 position);
    void Reset();
    i32 Insert(i32 position, i32 *matchPosition);
    void Delete(i32 position);
    LzssTree();

  protected:
    struct Node
    {
        i32 parent;
        i32 left;
        i32 right;
    };
    u8 window[LZSS_DICTSIZE];
    Node nodes[LZSS_DICTSIZE + 1];

  private:
    void Contract(i32 oldPosition, i32 newPosition);
    void Replace(i32 oldPosition, i32 newPosition);
    i32 FindPredecessor(i32 position);
};
ZUN_ASSERT_SIZE(LzssTree, 0x1a00c);
} // namespace th06
