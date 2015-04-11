
#include "precomp.h"

#include "Go/Block.h"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace Common;


namespace Go {

  namespace {
    // static const int block_allocate_each_count = 5000;
    // //int allocation_called_count = 0;
    // //static int CUSTOM_MEMORY_MAGIC_NUMBER = 0x10101010;

    // vector<char *> usable_blocks;
    // //vector<Block *> used_blocks;
    // //vector<int> usable_block_number;

    Board dummyRef(1);
  }

  void Block::deleteBlock(BlockPtr block) {
    //char *memory_head = reinterpret_cast<char *>(block)-sizeof(int);
    //assert(*reinterpret_cast<int *>(memory_head) == CUSTOM_MEMORY_MAGIC_NUMBER);
    block->~Block();
    //usable_blocks.push_back(memory_head);
    BlockMemoryAllocator::getInstance().release(block);
    //usable_blocks.push_back(reinterpret_cast<char *>(block));
  }

  Block::Block()
    : m_board(dummyRef)
    , m_color(FREE)
  {
  }

  Block::Block(Color color, Point anchor, const Board &board)
    : m_board(board)
    , m_color(color)
    , m_anchor(anchor)
      //, m_isSafe(false)
      //, m_has1Eye(false)
    , m_isDead(true)
    , m_refs(1)
    , m_stones()
    , m_liberties()
  {
    m_stones.insert(anchor, anchor);
  }

  void Block::initAllocator() {
    BlockMemoryAllocator::getInstance();
  }

  void Block::delete_this() {
    //cerr << "delete!!!!!!!!" << endl;
    if (!m_isDead) {
      m_isDead = true;
      deleteBlock(this);
    }
    m_refs = 0;
  }

  BlockPtr Block::createNewBlock(Color color, Point anchor, const Board &board)
  {
    // if (usable_blocks.size() == 0) {
    //   cerr << "!!!!!!!!!!!!!!!!Blocks Reallocation executed" << endl;
    //   usable_blocks.reserve(block_allocate_each_count);
    //   //used_blocks.reserve(used_blocks.size()+block_allocate_each_count);
    //   for (int i=0; i<block_allocate_each_count; i++) {
    //     char *block = new char[sizeof(Block)];
    //     //char *block = new char[sizeof(int)+sizeof(Block)];
    //     usable_blocks.push_back(block);
    //   }
    //   cerr << "Blocks Reallocation finished" << endl;
    // }
    
    // char *next_block = usable_blocks.back();
    // //usable_blocks.resize(usable_blocks.size()-1);
    // usable_blocks.pop_back();

    //int *magic_number_area = reinterpret_cast<int *>(next_block);
    //*magic_number_area = CUSTOM_MEMORY_MAGIC_NUMBER;

    //Block *newblock = new(next_block) Block(color, anchor, board);
    Block *newblock = new(BlockMemoryAllocator::getInstance().allocate()) Block(color, anchor, board);
    newblock->m_isDead = false;
    //assert(((char *)newblock)-sizeof(int) == next_block);
    //BlockPtr newptr(newblock, deleteBlock);
    BlockPtr newptr(newblock);

    return newptr;

    //shared_ptr<Block> p(new Block(color, anchor, board));
    //return p;
  }
/*
  std::shared_ptr<Block> Block::clone() const {
    shared_ptr<Block> newblock(createNewBlock(m_color, m_anchor, m_board));

    Block *pBlock = newblock.get();

    pBlock->m_isSafe = m_isSafe;
    pBlock->m_has1Eye = m_has1Eye;
    pBlock->m_stones = m_stones;
    pBlock->m_liberties = m_liberties;

    return newblock;
  }
*/
  BlockPtr Block::clone() const {
    BlockPtr newblock(createNewBlock(m_color, m_anchor, m_board));

    //Block *pBlock = newblock.get();
    Block *pBlock = newblock;

    pBlock->copyFrom(*this);
    pBlock->m_stones.insert(m_anchor, m_anchor);

    return newblock;
  }

  void Block::copyFrom(const Block &rhs) {
    //cerr << "Copy To " << this << " From " << &rhs << endl;
    this->m_color = rhs.m_color;
    this->m_anchor = rhs.m_anchor;
    this->m_isSafe = rhs.m_isSafe;
    this->m_has1Eye = rhs.m_has1Eye;
    this->m_stones = rhs.m_stones;
    this->m_liberties = rhs.m_liberties;
  }

  void Block::mergeFrom(const Block &block) {
    assert(block.getColor() == m_color);

    PointListConstIterator sto_it = block.m_stones.begin();
    for (; sto_it != block.m_stones.end(); ++sto_it) {
      this->addStone(*sto_it);
    }
    PointListConstIterator lib_it = block.m_liberties.begin();
    for (; lib_it != block.m_liberties.end(); ++lib_it) {
      this->addLiberty(*lib_it);
    }

    // update anchor and isSafe and has1Eye
  }

  void Block::removeBy(const Block &block) {
    assert(block.getColor() == m_color);

    PointListConstIterator sto_it = block.m_stones.begin();
    for (; sto_it != block.m_stones.end(); ++sto_it) {
      removeStone(*sto_it);
    }
    PointListConstIterator lib_it = block.m_liberties.begin();
    for (; lib_it != block.m_liberties.end(); ++lib_it) {
      if (!m_board.isAdjacentBlock(*lib_it, this)) {
        removeLiberty(*lib_it);
      }
    }

    // update anchor and isSafe and has1Eye
  }
}
