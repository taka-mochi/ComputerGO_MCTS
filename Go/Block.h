#pragma once

#include "precomp.h"
#include <vector>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <iostream>

#include "common.h"

#include "utility/SingletonMemoryPool.h"

namespace Go {

  class Board;

  class Block;
  //typedef std::shared_ptr<Block> BlockPtr;
  typedef Block * BlockPtr;
    
  class Block {

    //friend class BlockPtr;
    typedef Common::SingletonMemoryPool<Block, 5000> BlockMemoryAllocator;
    friend class Common::SingletonMemoryPool<Block, 5000>;

  private:
    const Board &m_board;
    Common::Color m_color;
    Common::Point m_anchor;
    bool m_isSafe;  // Is block marked as safe?

    bool m_has1Eye;

    bool m_isDead;

    int m_refs;

    //std::unordered_set<Common::Point> m_stones;
    //std::unordered_set<Common::Point> m_liberties;
    Common::PointList m_stones;
    Common::PointList m_liberties;

    Block();
    Block(Common::Color color, Common::Point anchor, const Board &board);

    void delete_this();
    static void deleteBlock(BlockPtr block);

  public:

    inline bool isDead() const {return m_isDead;}
    void incRef() {m_refs++;}
    void decRef() {m_refs--; if (m_refs <= 0) delete_this();}
    int refCount() const {return m_refs;}

    static void initAllocator();
    static BlockPtr createNewBlock(Common::Color color, Common::Point anchor, const Board &board);

    BlockPtr clone() const;
    void copyFrom(const Block &rhs);

    void mergeFrom(const Block &block);
    void removeBy(const Block &block);

    int getColor() const {return m_color;}

    inline void addStone(Common::Point p) {
      //if (std::find(m_stones.rbegin(), m_stones.rend(), p) == m_stones.rend()) {
      //  m_stones.push_back(p);
      //}
      assert(!m_isDead);
      m_stones.insert(p,p);
    }
    inline void removeStone(Common::Point p) {
      //Common::PointList::reverse_iterator it = std::find(m_stones.rbegin(), m_stones.rend(), p);
      //if (it != m_stones.rend()) {
      //  m_stones.erase(--(it.base()));
      //}
      assert(!m_isDead);
      m_stones.erase(p);
      //if (m_stones.size() == 0) delete_this();
    }
    inline void addLiberty(Common::Point p) {
      //Common::PointList::reverse_iterator it = std::find(m_liberties.rbegin(), m_liberties.rend(), p);
      //if (it == m_liberties.rend()) {
      //  m_liberties.push_back(p);
      //}
      assert(!m_isDead);
      m_liberties.insert(p,p);
    }
    inline void removeLiberty(Common::Point p) {
      //Common::PointList::reverse_iterator it = std::find(m_liberties.rbegin(), m_liberties.rend(), p);
      //if (it != m_liberties.rend()) {
      //m_liberties.erase(--(it.base()));
      //}
      assert(!m_isDead);
      m_liberties.erase(p);
    }

    inline const Common::PointList &getStones() const {assert(!m_isDead); return m_stones;}
    inline size_t getStoneCount() const {assert(!m_isDead); return m_stones.size();}
    inline const Common::PointList &getLiberties() const {assert(!m_isDead); return m_liberties;}
    inline size_t getLibertyCount() const {assert(!m_isDead); return m_liberties.size();}
  };
}
