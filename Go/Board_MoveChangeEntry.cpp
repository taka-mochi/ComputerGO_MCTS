#include "precomp.h"

#include "Go/Board.h"

//#include "utility/SingletonMemoryPool.h"

using namespace std;
using namespace Common;

namespace Go {

  typedef SingletonMemoryPool<Board::MoveChangeEntry, MAX_BOARD_SIZE*2*10> EntryMemoryAllocator;

  Board::MoveChangeEntry *Board::MoveChangeEntry::createNewEntry() {
    Board::MoveChangeEntry *entry = new(EntryMemoryAllocator::getInstance().allocate()) MoveChangeEntry();
    return entry;
  }
  void Board::MoveChangeEntry::releaseEntry(Board::MoveChangeEntry *entry) {
    entry->~MoveChangeEntry();
    EntryMemoryAllocator::getInstance().release(entry);
  }

  Board::MoveChangeEntry::MoveChangeEntry()
    : m_putPos(PASS)
    , m_putColor(FREE)
    , m_isFirst(true)
    , m_isNewPosition(true)
    , m_kouPos(PASS)
    , m_kouColor(FREE)
    , m_hash()
    , m_addedBlock(NULL)
  {
    for(int i=0; i<4; i++) {
      m_mergedBlocks[i] = NULL;;
      m_killedBlocks[i] = NULL;
      m_addedLiberties[i] = PASS;
    }
  }

  Board::MoveChangeEntry::~MoveChangeEntry() {
    SAFE_DECREF(m_addedBlock);
    m_addedBlock = NULL;
    for(int i=0; i<4; i++) {
      SAFE_DECREF(m_mergedBlocks[i]);
      m_mergedBlocks[i] = NULL;;
      SAFE_DECREF(m_killedBlocks[i]);
      m_killedBlocks[i] = NULL;
    }
  }

  void block_replaced_by(BlockPtr *replace_to, const BlockPtr *replaced_by, 
                         Board::BoardState::BlocksMapping &mapping) {

    typedef Board::BoardState::BlocksMapping Mapping;
    SAFE_DECREF(*replace_to);
    if (*replaced_by) {
      Mapping::iterator it = mapping.find(*replaced_by);
      if (it != mapping.end()) {
        *replace_to = it->second;
        it->second->incRef();
      } else {
        BlockPtr p = (*replaced_by)->clone();
        mapping[*replaced_by] = p;
        *replace_to = p;
        //p->incRef();
      }
    } else {
      *replace_to = NULL;
    }
  }

  Board::MoveChangeEntry &Board::MoveChangeEntry::copyFromWithMapping(const MoveChangeEntry &from, Board::BoardState::BlocksMapping &mapping) {
    m_putPos = from.m_putPos;
    m_putColor = from.m_putColor;
    m_isFirst = from.m_isFirst;
    m_isNewPosition = from.m_isNewPosition;
    m_kouPos = from.m_kouPos;
    m_kouColor = from.m_kouColor;
    block_replaced_by(&m_addedBlock, &from.m_addedBlock, mapping);
    for (int i=0; i<4; i++) {
      m_addedLiberties[i] = from.m_addedLiberties[i];
      //if (m_mergedBlocks[i] != NULL || from.m_mergedBlocks[i] != NULL) {
        block_replaced_by(&m_mergedBlocks[i], &from.m_mergedBlocks[i], mapping);
        //}
      //if (m_killedBlocks[i] != NULL || from.m_killedBlocks[i] != NULL) {
        block_replaced_by(&m_killedBlocks[i], &from.m_killedBlocks[i], mapping);
        //}
    }
    return *this;
  }
/*
  Board::MoveChangeEntry &Board::MoveChangeEntry::operator =(const MoveChangeEntry &entry) {
    m_putPos = entry.m_putPos;
    m_putColor = entry.m_putColor;
    m_isFirst = entry.m_isFirst;
    m_isNewPosition = entry.m_isNewPosition;
    m_kouPos = entry.m_kouPos;
    m_kouColor = entry.m_kouColor;
    block_replaced_by(&m_addedBlock, &entry.m_addedBlock);
    for (int i=0; i<4; i++) {
      m_addedLiberties[i] = entry.m_addedLiberties[i];
      block_replaced_by(&m_mergedBlocks[i], &entry.m_mergedBlocks[i]);
      block_replaced_by(&m_killedBlocks[i], &entry.m_killedBlocks[i]);
    }

    return *this;
  }
  Board::MoveChangeEntry::MoveChangeEntry(const MoveChangeEntry &entry)
    : m_putPos(PASS)
    , m_putColor(FREE)
    , m_isFirst(true)
    , m_isNewPosition(true)
    , m_kouPos(PASS)
    , m_kouColor(FREE)
    , m_addedBlock(NULL)
  {
    for(int i=0; i<4; i++) {
      m_mergedBlocks[i] = NULL;;
      m_killedBlocks[i] = NULL;
      m_addedLiberties[i] = PASS;
    }
    this->operator=(entry);
  }
*/
  void Board::MoveChangeEntry::copyVector(vector<MoveChangeEntry *> &copy_to, vector<MoveChangeEntry *> &copy_from, Board::BoardState::BlocksMapping &mapping) {

    if (copy_to.size() < copy_from.size()) {
      for (size_t i=copy_to.size(); i<copy_from.size(); i++)
        copy_to.push_back(createNewEntry());
    } else {
      for (int i=(signed)copy_to.size()-1; i>=(signed)copy_from.size(); i--) {
        releaseEntry(copy_to[i]);
      }
      copy_to.resize(copy_from.size());
    }

    for (size_t i=0; i<copy_from.size(); i++) {
      copy_to[i]->copyFromWithMapping(*copy_from[i], mapping);
    }
  }

}
