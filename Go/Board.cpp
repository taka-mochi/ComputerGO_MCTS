#include "precomp.h"

#include <iostream>
#include <sstream>
#include "Go/Board.h"

using namespace std;
using namespace Common;

//#define SAFE_DECREF(p) {if(p) p->decRef();}

namespace Go {

  Board::BoardState::BoardState(Board *board)    
    : m_boardOrig(board)
    , m_size(0)
    , m_putPos(0)
    , m_putColor(0)
    , m_kouPos(0)
    , m_usedList()
    , m_hash()
    , m_neighborCount()
    , m_neighborEmptyCount()
    , m_neighborWallCount()
    , m_belongBlocks()
    , m_isNewPosition(true)
    , m_board(NULL)
    , m_isFirst(NULL)
  {
  }

  Board::BoardState::BoardState(int size, Board *board)
    : m_boardOrig(board)
    , m_size(size+2)
    , m_putPos(0)
    , m_putColor(0)
    , m_kouPos(0)
    , m_usedList()
    , m_hash()
      //, m_capturedCount()
    , m_neighborCount()
    , m_neighborEmptyCount()
    , m_neighborWallCount()
    , m_belongBlocks()
    , m_isNewPosition(true)
    , m_board(new Color[(size+2)*(size+2)])
    , m_isFirst(new bool[(size+2)*(size+2)])
  {
    clear();
  }

  Board::BoardState::~BoardState()
  {
    for (size_t i=0; i<m_belongBlocks.size(); i++) {
      BlockPtr block = m_belongBlocks[i];
      SAFE_DECREF(block);
    }
    m_belongBlocks.clear();
    delete [] m_board;
    delete [] m_isFirst;
  }

  void Board::BoardState::copyFrom(const BoardState &rhs, BlocksMapping &old_to_new) {
    if (m_size != rhs.m_size) {
      delete [] m_board;
      delete [] m_isFirst;
      m_board = new Color[rhs.m_size*rhs.m_size];
      m_isFirst = new bool[rhs.m_size*rhs.m_size];
      assert(m_board);
      assert(m_isFirst);
    }
    assert(m_board);
    assert(m_isFirst);
    m_size = rhs.m_size;
    m_kouPos = rhs.m_kouPos;
    m_putPos = rhs.m_putPos;
    m_putColor = rhs.m_putColor;
    m_usedList = rhs.m_usedList;
    m_hash = rhs.m_hash;
    m_capturedCount[BLACK] = rhs.m_capturedCount[BLACK];
    m_capturedCount[WHITE] = rhs.m_capturedCount[WHITE];
    for (int i=0; i<4; i++) m_neighborCount[i] = rhs.m_neighborCount[i];
    m_neighborEmptyCount = rhs.m_neighborEmptyCount;
    m_isNewPosition = rhs.m_isNewPosition;
    // safe copy of belong blocks
    m_belongBlocks.resize(rhs.m_belongBlocks.size());
    BlocksMapping &from_old_block_to_new_block = old_to_new;
    for (size_t i=0; i<m_belongBlocks.size(); i++) {
      BlockPtr block = rhs.m_belongBlocks[i];
      if (block) {
        //Block *p = block.get();
        Block *p = block;
        BlockPtr newp = NULL;
        if (from_old_block_to_new_block.find(p) == from_old_block_to_new_block.end()) {
          newp = block->clone();
          from_old_block_to_new_block[p] = newp;
          
        } else {
          newp = from_old_block_to_new_block[p];
          newp->incRef();
        }
        SAFE_DECREF(m_belongBlocks[i]);
        m_belongBlocks[i] = newp;
      } else {
        SAFE_DECREF(m_belongBlocks[i]);
        m_belongBlocks[i] = BlockPtr(NULL);
      }
    }
    memcpy(m_board, rhs.m_board, sizeof(Color)*m_size*m_size);
    memcpy(m_isFirst, rhs.m_isFirst, sizeof(bool)*m_size*m_size);
  }

  void Board::BoardState::clear() {
    m_usedList = 0;
    for (int i=1; i<m_size-1; i++) {
      for (int j=1; j<m_size-1; j++) {
        m_board[rawXYtoPoint(i,j)] = FREE;
      }
    }
    for (int i=0; i<m_size; i++) {
      Point p1 = rawXYtoPoint(i, 0);
      Point p2 = rawXYtoPoint(i, m_size-1);
      m_board[p1] = WALL;
      m_board[p2] = WALL;
      m_usedList.set(p1);
      m_usedList.set(p2);
    }
    for (int j=0; j<m_size; j++) {
      Point p1 = rawXYtoPoint(0, j);
      Point p2 = rawXYtoPoint(m_size-1, j);
      m_board[p1] = WALL;
      m_board[p2] = WALL;
      m_usedList.set(p1);
      m_usedList.set(p2);
    }
    for (int i=0; i<m_size*m_size; i++) {
      m_isFirst[i] = true;
    }
    m_isNewPosition = true;
    m_capturedCount[WHITE] = m_capturedCount[BLACK] = 0;
    m_neighborCount[BLACK] = vector<int>();
    m_neighborCount[BLACK].resize(m_size*m_size, 0);
    m_neighborCount[WHITE] = vector<int>();
    m_neighborCount[WHITE].resize(m_size*m_size, 0);
    m_neighborEmptyCount = vector<int>();
    m_neighborEmptyCount.resize(m_size*m_size, 4);
    for (int x=1; x<m_size-2-1; x++) {
      m_neighborEmptyCount[xyToPoint(x,0)] = 3;
      m_neighborEmptyCount[xyToPoint(x,m_size-2-1)] = 3;
    }
    for (int y=1; y<m_size-2-1; y++) {
      m_neighborEmptyCount[xyToPoint(0, y)] = 3;
      m_neighborEmptyCount[xyToPoint(m_size-2-1, y)] = 3;
    }
    m_neighborEmptyCount[xyToPoint(0,0)] = m_neighborEmptyCount[xyToPoint(0,m_size-2-1)] = 
      m_neighborEmptyCount[xyToPoint(m_size-2-1,0)] = m_neighborEmptyCount[xyToPoint(m_size-2-1,m_size-2-1)] = 2;
    for (size_t i=0; i<m_belongBlocks.size(); i++) {
      SAFE_DECREF(m_belongBlocks[i]);
    }
    m_belongBlocks.clear();
    m_belongBlocks.resize(m_size*m_size, BlockPtr(NULL));

    m_putPos = POINT_NULL; m_putColor = FREE;
    m_hash.fromBoard(*m_boardOrig);
  }
  

  Board::Board(int size, Color *init_array)
    : m_originalSize(size)
    , m_changeHistory()
    , m_state(size, this)
    , m_snapshotState(this)
  {
    s_allDirs[3] = DIR_LEFT = -1;
    s_allDirs[4] = DIR_RIGHT = +1;
    s_allDirs[1] = DIR_UP = -m_state.m_size;
    s_allDirs[6] = DIR_DOWN = m_state.m_size;
    s_allDirs[0] = DIR_LEFTUP = DIR_UP + DIR_LEFT;
    s_allDirs[2] = DIR_RIGHTUP = DIR_UP + DIR_RIGHT;
    s_allDirs[5] = DIR_LEFTDOWN = DIR_DOWN + DIR_LEFT;
    s_allDirs[7] = DIR_RIGHTDOWN = DIR_DOWN + DIR_RIGHT;

    s_dirZ[0] = DIR_LEFT;
    s_dirZ[1] = DIR_RIGHT;
    s_dirZ[2] = DIR_UP;
    s_dirZ[3] = DIR_DOWN;

    clear();
    if (init_array) {
      initFromArray(init_array);
    }
  }
/*
  Board::Board(const Board &rhs)
  {
    copyFrom(rhs);
  }
*/
  Board::~Board() {
    releaseMoveChangeEntryVector(m_changeHistory);
    releaseMoveChangeEntryVector(m_snapshotHistory);
  }
  
  void Board::initFromArray(const Color *array) {
    assert(m_state.m_board);
    clear();
    assert(m_state.m_board);
    for (int x=0; x<m_originalSize; x++) {
      for (int y=0; y<m_originalSize; y++) {
        Point z = xyToPoint(x,y);
        if (array[z] == WHITE || array[z] == BLACK) {
          //cout << m_state.m_board << endl;
          put(z, array[z]);
        }
      }
    }
    releaseMoveChangeEntryVector(m_changeHistory);
    m_changeHistory.reserve(MAX_BOARD_SIZE);
  }
/*
  void Board::copyFrom(const Board &rhs) {
    m_state.copyFrom(rhs.m_state);
    // this copy is secure because copy constructor is defined in MoveChangeEntry
    
//m_changeHistory = rhs.m_changeHistory; 
  }
*/
  void Board::clear() {
    m_state.clear();
    releaseMoveChangeEntryVector(m_changeHistory);
    m_changeHistory.reserve(MAX_BOARD_SIZE);
  }

  void Board::releaseMoveChangeEntryVector(std::vector<MoveChangeEntry *> &history) {
    for (size_t i=0; i<history.size(); i++) {
      MoveChangeEntry::releaseEntry(history[i]);
    }
    history.clear();
  }

  Board::PutType Board::put(Point z, Color color, bool withoutLegalCheck) {
    checkConsistency();
    if (z == PASS) {
      // pass
      Board::MoveChangeEntry &new_ent = createNewHistoryEntry();
      new_ent.m_putPos = m_state.m_putPos = z;
      new_ent.m_putColor = m_state.m_putColor = color;
      saveState(new_ent);
      if (m_state.m_kouPos > 0) m_state.m_hash.xorKou(m_state.m_kouPos, color);
      m_state.m_hash.xorToPlay(color); m_state.m_hash.xorToPlay(flipColor(color));
      m_state.m_kouPos = PASS;
      return PUT_LEGAL;
    }
    
    //int around_dame[4], around_stones[4], around_colors[4];
    Color enemy_color = flipColor(color);
    PutType put_type;
    if (!withoutLegalCheck) {
      put_type = checkLegalHand(z, color, enemy_color);
      if (put_type != PUT_LEGAL) {
        return put_type;
      }
    } else {
      put_type = PUT_LEGAL;
    }

    // legal hand -> create new move change entry
    m_state.m_putPos = z;
    m_state.m_putColor = color;
    MoveChangeEntry &entry = createNewHistoryEntry();

    // execute putting a stone
    executePutAndUpdate(entry, z, color, enemy_color);
    m_state.m_hash.xorToPlay(color); m_state.m_hash.xorToPlay(flipColor(color)); // invert to play

    checkConsistency();
    return PUT_LEGAL;
  }

  void Board::enumerateFreeMoves(Point *point_array, int &point_count, Color turn) const {
    point_count = 0;
//     int max_size = m_state.m_size*m_state.m_size - m_state.m_size;
//     for (int i=m_state.m_size; i<max_size; i++) {
//       if (!m_state.m_usedList[i]) {
//         point_array[point_count++] = i;
//       }
//     }
    Point p = xyToPoint(0,0);
    for (int y=0; y<m_state.m_size-2; y++) {
      for (int x=0; x<m_state.m_size-2; x++, p++) {
        if (!isColor(p, FREE)) continue;
        point_array[point_count++] = p;
      }
      p += 2;
    }
  }

  Board::PutType Board::checkLegalHand(Point z, Color color, Color enemy_color) const {

    // PASS は常にlegal
    if (z == PASS) return PUT_LEGAL;

    // 既に石がある
    if (m_state.m_board[z] != FREE) return PUT_NOT_FREE;

    // simple ko rule
    if (z == m_state.m_kouPos) return PUT_KOU;

    //int space_count = 0;
    int wall_count = 0;

    //int ko_candidate = 0;
    //int get_sum = 0;
    //bool will_get = false;
    int myself_safe_count = 0;

    //PerformanceCountTool::time_counter_set check_get_sum(clock());

    // NeighborBlockSet around_blocks;
    // getNeighborBlocks(z, around_blocks);
    // NeighborBlockSet::iterator it = around_blocks.begin();
    // for (; it!=around_blocks.end(); ++it) {
    //   const shared_ptr<Block> &block = *it;
    //   if (block->getColor() == enemy_color && block->getLibertyCount() == 1) {
    //     get_sum += block->getStoneCount();
    //     //ko_candidate = around_z;
    //   }
    // }

    ///////////
    // timer //
    ///////////
    //check_get_sum.setEnd(clock());
    //check_get_sum_timer.add_time_counter(check_get_sum);
    //cout << "check_get_sum_timer.diff = " << check_get_sum.diff << endl;

    ///////////
    // timer //
    ///////////
    //PerformanceCountTool::time_counter_set check_safe_sum(clock());

    if (m_state.m_neighborEmptyCount[z]>=1) return PUT_LEGAL;


    for (int i=0; i<4; i++) {
      Point around_z = z+s_dirZ[i];
      Color c = m_state.m_board[around_z];

      assert (c != FREE);
      if (c == WALL) {wall_count++; continue;}

      Block *block = m_state.m_belongBlocks[around_z];//.get();
      if (block->getColor() == color) {
        if (block->getLibertyCount() >= 2) {
          myself_safe_count++;
          if (Rules::isPutEyeAllowed()) return PUT_LEGAL;
        }
      } else if (block->getLibertyCount() == 1) {
        // can capture
        return PUT_LEGAL;
      }
    }

    //if (will_get || space_count > 0) return PUT_LEGAL;

    // TODO: remove magic numbers

    // 自殺手
    if (myself_safe_count == 0) return PUT_SUICIDE;
    // 眼 (4方向が自分 or 壁、で自殺手ではない)
    if (!Rules::isPutEyeAllowed() && (wall_count + myself_safe_count == 4)) {
      if (!isFalseEye(z, color)) return PUT_EYE;
    }

/*
  if (isSuicide(z, color)) {
  return PUT_SUICIDE;
  }
*/
    return PUT_LEGAL;
  }

  bool Board::isFalseEye(Point p, Color c) const {
    int dirCounts[4] = {0};
    Color enemy = flipColor(c);
    dirCounts[m_state.m_board[p+DIR_RIGHTDOWN]]++;
    dirCounts[m_state.m_board[p+DIR_RIGHTUP]]++;
    dirCounts[m_state.m_board[p+DIR_LEFTDOWN]]++;
    dirCounts[m_state.m_board[p+DIR_LEFTUP]]++;

    // if enemyCount - (spaceCount + myCount) >= 0, this is false eye
    // this equation can be written as follow:
    //   2 * enemyCount + wallCount >= 4
    // because:
    //   enemyCount + wallCount + spaceCount + myCount = 4
    //return dirCounts[enemy] >= dirCounts[FREE] + dirCounts[c];
    //return 2 * dirCounts[enemy] + dirCounts[WALL] >= 4;
    return (dirCounts[enemy] << 1) + dirCounts[WALL] >= 4;
  }

  bool Board::isSuicide(Point p, Color c) const {
    if (m_state.m_neighborEmptyCount[p] > 0) return false;
    Color enemy = flipColor(c);

    for (int i=0; i<4; i++) {
      Point z = p+s_dirZ[i];
      Color zc = m_state.m_board[z];
      assert(zc!=FREE);
      if (zc == c && m_state.m_belongBlocks[z]->getLibertyCount()>1) {
        return false;
      }
      if (zc == enemy && m_state.m_belongBlocks[z]->getLibertyCount()==1) {
        return false;
      }
    }
    return true;
  }

  void Board::executePutAndUpdate(MoveChangeEntry &entry, Point z, Color color, Color enemy_color) {
    addStone(z, color);
    m_state.m_putPos = z;
    if (m_state.m_kouPos > 0) {
      m_state.m_hash.xorKou(m_state.m_kouPos, color);
    }
    m_state.m_kouPos = PASS;
    bool wasFirstPut = m_state.m_isFirst[z];
    m_state.m_isFirst[z] = false;

    NeighborBlockSet neighborBlocks;

    if (getNeighborCount(z, BLACK) > 0 || getNeighborCount(z, WHITE) > 0) {
      removeLibertyAndKill(z, enemy_color, entry, neighborBlocks);
    }

    //if (entry.m_killedBlocks[0].get() != NULL) {
    if (entry.m_killedBlocks[0] != NULL) {
      // some stones were captured
      // repetition can be happened, but if this put is FIRST PUT, repetition cannot be happened
      m_state.m_isNewPosition = m_state.m_isNewPosition && wasFirstPut;
    }
    updateBlockAfterAddStone(z, color, entry, neighborBlocks);
    if (m_state.m_kouPos != PASS) {
      //Block *b = m_state.m_belongBlocks[z].get();
      Block *b = m_state.m_belongBlocks[z];
      assert(b);
      if (b->getStoneCount() > 1 || b->getLibertyCount() > 1) {
        m_state.m_kouPos = PASS;
      } else {
        m_state.m_hash.xorKou(m_state.m_kouPos, flipColor(color));
      }
    }
  }

  void Board::addStone(Point p, Color c) {
    assert(m_state.m_board[p]==FREE);
    m_state.m_board[p] = c;
    m_state.m_hash.xorMove(p,FREE);
    m_state.m_hash.xorMove(p,c);
    //m_state.m_usedList.set(p);
    // if there were some other information, update them (such as neighbors)
    vector<int> &neighborCount = m_state.m_neighborCount[c];
    for (int i=0; i<4; i++) {
      --m_state.m_neighborEmptyCount[p+s_dirZ[i]];
      ++neighborCount[p+s_dirZ[i]];
    }
  }

  void Board::addLibertiesToAdjacentBlocks(Point p) {
    for (int i=0; i<4; i++) {
      BlockPtr b = m_state.m_belongBlocks[p+s_dirZ[i]];
      if (b) {
        b->addLiberty(p);
      }
    }
  }

  void Board::addLibertiesToAdjacentBlocks(Point p, Color c) {
    for (int i=0; i<4; i++) {
      BlockPtr b = m_state.m_belongBlocks[p+s_dirZ[i]];
      if (b && b->getColor() == c) {
        b->addLiberty(p);
      }
    }
  }

  void Board::removeStone(Point p) {
    assert(m_state.m_board[p]!=FREE);
    Color c = m_state.m_board[p];
    m_state.m_hash.xorMove(p, m_state.m_board[p]);
    m_state.m_hash.xorMove(p, FREE);
    m_state.m_board[p] = FREE;
    //m_state.m_usedList.reset(p);
    // if there were some other information, update them (such as neighbors)
    vector<int> &neighborCount = m_state.m_neighborCount[c];
    for (int i=0; i<4; i++) {
      ++m_state.m_neighborEmptyCount[p+s_dirZ[i]];
      --neighborCount[p+s_dirZ[i]];
    }
  }

  void Board::removeLibertiesToAdjacentBlocks(Point p) {
    for (int i=0; i<4; i++) {
      BlockPtr b = m_state.m_belongBlocks[p+s_dirZ[i]];
      if (b) {
        b->removeLiberty(p);
      }
    }
  }

  void Board::removeLibertiesToAdjacentBlocks(Point p, Color c) {
    for (int i=0; i<4; i++) {
      BlockPtr b = m_state.m_belongBlocks[p+s_dirZ[i]];
      if (b && b->getColor() == c) {
        b->removeLiberty(p);
      }
    }
  }
  
  bool Board::isAdjacentBlock(Point p, const Block* b) const {
    for (int i=0; i<4; i++) {
      if (m_state.m_belongBlocks[p+s_dirZ[i]] == b) {
        return true;
      }
    }
    return false;
  }

  // create a new Block for single stone
  void Board::createNewSingleBlock(Point posZ, Color color, MoveChangeEntry &changesEntry) {    
    assert(isColor(posZ, color));

    Point p(posZ);
    BlockPtr newBlock(Block::createNewBlock(color, p, *this));
    
    for (int i=0, j=0; i<4; i++) {
      Point z = posZ+s_dirZ[i];
      if (isColor(z, FREE)) {
        newBlock->addLiberty(z);
        changesEntry.m_addedLiberties[j++] = z;
      }
    }

    changesEntry.m_addedBlock = newBlock;
    m_state.m_belongBlocks[posZ] = newBlock;
    newBlock->incRef();
  }

  // update block information after adding stone
  void Board::updateBlockAfterAddStone(Point posZ, Color color, MoveChangeEntry &changesEntry, NeighborBlockSet &neighbors) {
    assert(isColor(posZ, color));

    //if (getNeighborCount(posZ, color) == 0) {
    if (neighbors.size() == 0) {
      // a block with single stone
      createNewSingleBlock(posZ, color, changesEntry);      
    } else {
      // merge a stone with blocks already exist
      //NeighborBlockSet neighbors;
      //getNeighborBlocks(posZ, color, neighbors);
      assert(neighbors.size()>0);

      //NeighborBlockSet::Iterator it = neighbors.begin();
      int j; size_t k; size_t largestIndex = 0;
      BlockPtr main_block = neighbors[0]; size_t largestSize = main_block->getStoneCount();
      for (k=1; k<neighbors.size(); k++) {
        if (neighbors[k]->getStoneCount() > largestSize) {
          main_block = neighbors[k];
          largestSize = main_block->getStoneCount();
          largestIndex = k;
        }
      }
      changesEntry.m_addedBlock = main_block;
      changesEntry.m_addedBlock->incRef();
      addStoneToBlockAndUpdate(posZ, color, main_block, changesEntry);
      for (j=0,k=0; k<neighbors.size(); ++k) {
        if (k==largestIndex) continue;

        BlockPtr block = neighbors[k];
        changesEntry.m_mergedBlocks[j++] = block;
        block->incRef();
        mergeTwoBlock(main_block, block);
      }
    }
  }

  void Board::addStoneToBlockAndUpdate(Point posZ, Color color, BlockPtr &block, MoveChangeEntry &changesEntry) {
    assert(isColor(posZ, color));
    //assert(m_state.m_belongBlocks[posZ]==NULL);

    //const PointList &liberties = block->getLiberties();

    for (int i=0, j=0; i<4; i++) {
      Point currentZ = posZ + s_dirZ[i];

      if (isColor(currentZ, FREE) && !isAdjacentBlock(currentZ, block)) {
        block->addLiberty(currentZ);
        changesEntry.m_addedLiberties[j++] = currentZ;
      }
    }
    block->addStone(posZ);
    m_state.m_belongBlocks[posZ] = block;
    block->incRef();
  }

  void Board::mergeTwoBlock(BlockPtr mergeTo, const BlockPtr& mergeFrom) {
    assert(mergeTo); assert(mergeFrom);
    mergeTo->mergeFrom(*mergeFrom);
    const PointList &stones = mergeFrom->getStones();
    for (PointListConstIterator stone_it = stones.begin();
         stone_it != stones.end(); ++stone_it ) {

      mergeTo->incRef();
      SAFE_DECREF(m_state.m_belongBlocks[*stone_it]);
      m_state.m_belongBlocks[*stone_it] = mergeTo;
    }
  }

  // remove liberty and kill stones if possible
  void Board::removeLibertyAndKill(Point posZ, Color enemy_color, MoveChangeEntry &changesEntry, NeighborBlockSet &neighborsBuffer) {
    assert(m_state.m_board[posZ] == flipColor(enemy_color));

    NeighborBlockSet blocks;
    Point pos(posZ);
    getNeighborBlocks(pos, blocks);
    //NeighborBlockSet::Iterator it = blocks.begin();
    
    size_t k=0;
    for (int j=0; k<blocks.size(); k++) {
      BlockPtr b = blocks[k];
      b->removeLiberty(pos);

      if (b->getColor() == enemy_color) {
        if (b->getLibertyCount() == 0) {
          // remove block "b"
          if (b->getStoneCount() == 1) {
            m_state.m_kouPos = *b->getStones().begin();
          }
          m_state.m_capturedCount[enemy_color] += b->getStoneCount();
          changesEntry.m_killedBlocks[j++] = b;
          b->incRef();
          removeBlock(b);
        }
      } else {
        neighborsBuffer.push_back(b);
      }
    }
  }

  // remove the block
  void Board::removeBlock(BlockPtr block) {
    const PointList &stones = block->getStones();
    PointListConstIterator it = stones.begin();

    int color = block->getColor();
    int oppColor = Board::flipColor(color);

    for (; it!=stones.end(); ++it) {
      Point z = *it;
      removeStone(z);
      //m_state.m_board[z] = FREE;
      SAFE_DECREF(m_state.m_belongBlocks[z]);
      m_state.m_belongBlocks[z] = BlockPtr(NULL);
      addLibertiesToAdjacentBlocks(z, oppColor);
    }
  }

  ///////////////////////////////////
  // Beginning of Methods for Undo //
  ///////////////////////////////////
  bool Board::undo() {
    checkConsistency();
    if (m_changeHistory.size()>0) {
      MoveChangeEntry *entry = m_changeHistory.back();
      restoreState(*entry);
      updateBlockAfterUndoChanges(*entry);
      MoveChangeEntry::releaseEntry(entry);
      m_changeHistory.pop_back();
      checkConsistency();
      return true;
    } else {
      return false;
    }
  }

  Board::MoveChangeEntry &Board::createNewHistoryEntry() {
    // legal hand -> create new move change entry
    MoveChangeEntry *new_entry = MoveChangeEntry::createNewEntry();
    m_changeHistory.push_back(new_entry);
    saveState(*new_entry);
    return *new_entry;
  }
  
  void Board::saveState(MoveChangeEntry &entry) {
    entry.m_kouPos = m_state.m_kouPos;
    entry.m_putColor = m_state.m_putColor;
    entry.m_putPos = m_state.m_putPos;
    entry.m_isNewPosition = m_state.m_isNewPosition;
    entry.m_hash = m_state.m_hash;
    if (entry.m_putPos != PASS) {
      entry.m_isFirst = m_state.m_isFirst[m_state.m_putPos];
    }
  }

  void Board::restoreState(const MoveChangeEntry &entry) {
    m_state.m_hash = entry.m_hash;
    m_state.m_kouPos = entry.m_kouPos;
    m_state.m_putColor = entry.m_putColor;
    m_state.m_putPos = entry.m_putPos;
    m_state.m_isNewPosition = entry.m_isNewPosition;
    if (m_state.m_putPos != PASS) {
      m_state.m_isFirst[m_state.m_putPos] = entry.m_isFirst;
    }
  }

  void Board::restoreKill(BlockPtr block, Color c) {
    Color oppColor = flipColor(c);

    if (block->isDead()) cerr << "----------------- accessing to dead block!!! 1 -----------------" << endl;

    const PointList &stones = block->getStones();
    m_state.m_capturedCount[c] -= block->getStoneCount();
    for (PointListConstIterator it = stones.begin();
         it!=stones.end(); ++it) {
      addStone(*it, c);
      BlockPtr old = m_state.m_belongBlocks[*it];
      block->incRef();
      SAFE_DECREF(old);
      m_state.m_belongBlocks[*it] = block;
      removeLibertiesToAdjacentBlocks(*it, oppColor);
    }
  }

  void Board::updateBlockAfterUndoChanges(const MoveChangeEntry &entry) {
    Point p = entry.m_putPos;
    Color c = entry.m_putColor;
    // m_state.m_hash.xorToPlay(c);
    // m_state.m_hash.xorToPlay(flipColor(c));
    if (p == PASS) {
      return;
    }

    // undo adding stone
    SAFE_DECREF(m_state.m_belongBlocks[p]);
    removeStone(p);
    m_state.m_belongBlocks[p] = BlockPtr(NULL);
    
    // undo adding blocks
    BlockPtr block = entry.m_addedBlock;
    if (block->isDead()) cerr << "----------------- accessing to dead block!!! 2 -----------------" << endl;
    block->removeStone(p);
    for (int i=0; entry.m_addedLiberties[i] != PASS && i<4; i++) {
      block->removeLiberty(entry.m_addedLiberties[i]);
    }

    const PointList &stones = block->getStones();
    for (PointListConstIterator it = stones.begin();
         it != stones.end(); ++it) {
      BlockPtr old = m_state.m_belongBlocks[*it];
      block->incRef();
      SAFE_DECREF(old);
      m_state.m_belongBlocks[*it] = block;
    }
    //SAFE_DECREF(entry.m_addedBlock);
        
    // 2 or more blocks were merged
    for (int i=0; entry.m_mergedBlocks[i] && i<4; i++) {
      BlockPtr b = entry.m_mergedBlocks[i];

      if (b->isDead()) cerr << "----------------- accessing to dead block!!! 3 -----------------" << endl;

      const PointList &stones = b->getStones();
      for (PointListConstIterator it = stones.begin();
           it != stones.end(); ++it) {
        BlockPtr old = m_state.m_belongBlocks[*it];
        b->incRef();
        SAFE_DECREF(old);
        m_state.m_belongBlocks[*it] = b;
      }
      block->removeBy(*b);

      //SAFE_DECREF(entry.m_mergedBlocks[i]);
    }

    // undo kill
    for (int i=0; entry.m_killedBlocks[i] && i<4; i++) {
      restoreKill(entry.m_killedBlocks[i], flipColor(c));
      //SAFE_DECREF(entry.m_killedBlocks[i]);
    }

    // add liberties
    addLibertiesToAdjacentBlocks(p);

    m_state.m_hash = entry.m_hash;
  }

  void Board::takeSnapshot() {
    BoardState::BlocksMapping mapping;
    m_snapshotState.copyFrom(m_state, mapping);
    MoveChangeEntry::copyVector(m_snapshotHistory, m_changeHistory, mapping);
  }
  void Board::restoreStateFromSnapshot() {
    BoardState::BlocksMapping mapping;
    m_state.copyFrom(m_snapshotState, mapping);
    MoveChangeEntry::copyVector(m_changeHistory, m_snapshotHistory, mapping);
  }

  /////////////////////////////
  // End of Methods for Undo //
  /////////////////////////////


  // return 1 if black wins and turn is black. return -1 if white wins and turn is white
  // otherwise, return 0
  double Board::countScore(Color turn_color) {
    double black = 0.0, white = 0.0;
    
    countScoreDetail(black, white);
  
    double score = black - white;

    //const double inverse_score_max = 0.5 * 0.0001;
    //const double inverse_score_min = -inverse_score_max;
    //double inverse_score_rate = ( 0.5 * score / (m_originalSize*m_originalSize)) * 0.0001 - inverse_score_min;
    //const double max_size = (1+inverse_score_max) - (0+inverse_score_min);
    //const double max_size_inv = 1.0 / max_size;
    //cerr << "black = " << black << " white = " << white << " inv_score_rate = " << inverse_score_rate << endl;
    
    //double win = 0 + inverse_score_rate;
    //if (score > 0) win = 1 + inverse_score_rate;  // black wins
    double win = 0;
    if (score > 0) win = 1;
    if (turn_color == WHITE) win = -win; // turn is white. the result should be reversed
    //win *= max_size_inv;
    //cerr << "max_inv = " << max_size_inv << " win = " << win << endl;
    return win;
  }

  void Board::countScoreDetail(double &black_score, double &white_score) {
    //int score = 0;
    int rest_stones_for_each_kind[3] = {0};

    black_score = white_score = 0.0;

    for (int x=0; x<m_originalSize; x++) {
      for (int y=0; y<m_originalSize; y++) {
        Point z = xyToPoint(x, y);
        Color c = m_state.m_board[z];
        if (c != FREE) {
          if (m_state.m_belongBlocks[z]->getLibertyCount() <= 1) {
            // liberty is 1 or less than 1. so this block is dead
            rest_stones_for_each_kind[flipColor(c)]++;
          } else {
            rest_stones_for_each_kind[c]++;
          }
        } else {
          // 空点に対する処理
          int around_kinds[4] = {0};
          for (int i=0; i<4; i++) around_kinds[m_state.m_board[z+s_dirZ[i]]]++;
          if (around_kinds[BLACK]>0 && around_kinds[WHITE]==0) rest_stones_for_each_kind[BLACK]++; // 黒だけに囲われている
          if (around_kinds[WHITE]>0 && around_kinds[BLACK]==0) rest_stones_for_each_kind[WHITE]++; // 白だけに囲われている
        }
      }
    }

    // 石の数を評価に入れる
    black_score = rest_stones_for_each_kind[BLACK];
    white_score = rest_stones_for_each_kind[WHITE] + Rules::getKomi();
  }

  int Board::getCapturedCount(Color turn) const {
    return m_state.m_capturedCount[turn];
  }

  void Board::getNeighborBlocks(Point p, NeighborBlockSet &gotBlocks) const {
    Point z = p;
    gotBlocks.clear();
    for (int i=0; i<4; i++ ) {
      BlockPtr b = m_state.m_belongBlocks[z+s_dirZ[i]];
      if (b && !gotBlocks.contains(b)) {
        gotBlocks.push_back(b);
      }
    }
  }
  void Board::getNeighborBlocks(Point p, Color color, NeighborBlockSet &gotBlocks) const {
    assert (color == BLACK || color == WHITE);
    Point z = p;
    gotBlocks.clear();
    for (int i=0; i<4; i++ ) {
      if (m_state.m_board[z+s_dirZ[i]] != color) continue;

      BlockPtr b = m_state.m_belongBlocks[z+s_dirZ[i]];
      if (!gotBlocks.contains(b)) {
        gotBlocks.push_back(b);
      }
    }
  }

  bool Board::isRepeatedPosition() const {

    int changedCount = 0;

    const static Color offset = WHITE-BLACK > 0 ? WHITE-BLACK : BLACK-WHITE;
    std::bitset<MAX_BOARD_SIZE> changes[2];
    // changes[offsetedWhite] => white's changes
    // changes[offsetedBlack] => black's changes

    for (int moveNumber = (int)m_changeHistory.size()-1; moveNumber>=0; moveNumber--) {
      //cerr << "access to changeHistory[" << moveNumber << "]" << endl;
      const MoveChangeEntry &entry = *m_changeHistory[moveNumber];
      Point p = entry.m_putPos;

      if (p != PASS) {
        if( isColor(p, entry.m_putColor) && entry.m_isFirst) {
          // Entry (moveNumber-th move) put ENTRY.M_PUTCOLOR on P at first time,
          // and now there is PUTCOLOR on P.
          // However, this entry is first put, so all erlier positions are empty at P
          return false;
        }

        changes[entry.m_putColor-offset].flip(entry.m_putPos);
        changedCount++;

        for (int i=0; i<4 && entry.m_killedBlocks[i]; i++) {
          const PointList &stones = entry.m_killedBlocks[i]->getStones();
          Color offsetedKilledColor = entry.m_killedBlocks[i]->getColor() - offset;
          for (PointListConstIterator it = stones.begin();
               it != stones.end(); ++it) {
            changes[offsetedKilledColor].flip(*it);
            changedCount++;
          }
        }
      }

      // changesが発生しているが、同じ点に対して行われた操作が全て偶数回（置く→取られる）であれば、同じところに戻っている
      if (changedCount > 0 && flipColor(entry.m_putColor) == m_state.m_putColor &&
          entry.m_kouPos == m_state.m_kouPos &&
          !changes[0].any() && !changes[1].any()) {
        return true;
      }
    }
    return false;
  }

  void Board::checkConsistency() const {
#ifndef NDEBUG
/*
    for (int x=0; x<m_originalSize; x++) {
      for (int y=0; y<m_originalSize; y++) {
        Point p = xyToPoint(x,y);
        if (m_state.m_board[p] != FREE) {
          assert(m_state.m_belongBlocks.at(p));
          unordered_map<Color, vector<int> >::const_iterator it = m_state.m_neighborCount.find(m_state.m_board[p]);
          assert(it!=m_state.m_neighborCount.end());
          const vector<int> &neighborcount = it->second;
          assert(neighborcount.at(p+DIR_LEFT)>0);
          assert(neighborcount.at(p+DIR_RIGHT)>0);
          assert(neighborcount.at(p+DIR_UP)>0);
          assert(neighborcount.at(p+DIR_DOWN)>0);
        } else {
          assert(!m_state.m_belongBlocks.at(p));
          assert(m_state.m_neighborEmptyCount.at(p+DIR_LEFT)>0);
          assert(m_state.m_neighborEmptyCount.at(p+DIR_RIGHT)>0);
          assert(m_state.m_neighborEmptyCount.at(p+DIR_UP)>0);
          assert(m_state.m_neighborEmptyCount.at(p+DIR_DOWN)>0);
        }

        if (m_state.m_belongBlocks.at(p)) {
          assert(m_state.m_board[p]!=FREE);
        } else {
          assert(m_state.m_board[p]==FREE);
        }
      }
    }
*/
#endif // #ifndef NDEBUG
  }

  void Board::createShowString(std::string &s) const {
    std::stringstream ss;
    static const string str [] = {
      "+", 
      "o", 
      "*", 
      "#"
    };
    static const string row [] = {
      "A", "B", "C", "D", "E", 
      "F", "G", "H", "J", 
      "K", "L", "M", "N", "O", 
      "P", "Q", "R", "S", "T", 
    };
    ss << "  ";
    for (int i=0; i<m_originalSize; i++) {
      ss << row[i];
    }
    ss << endl;

    for (int i=0; i<m_originalSize; i++) {
      ss << ((i+1)/10) << (i+1)%10;
      for (int j=0; j<m_originalSize; j++) {
        ss << str[m_state.m_board[xyToPoint(j, i)]];
      }
      ss << endl;
    }
    s = ss.str();
  }

  void Board::createInvertedShowString(std::string &s) const {
    std::stringstream ss;
    static const string str [] = {
      "+", 
      "o", 
      "*", 
      "#"
    };
    static const string row [] = {
      "A", "B", "C", "D", "E", 
      "F", "G", "H", "J", 
      "K", "L", "M", "N", "O", 
      "P", "Q", "R", "S", "T", 
    };
    ss << "  ";
    for (int i=0; i<m_originalSize; i++) {
      ss << row[i];
    }
    ss << endl;

    for (int i=m_originalSize-1; i>=0; i--) {
      ss << ((i+1)/10) << (i+1)%10;
      for (int j=0; j<m_originalSize; j++) {
        ss << str[m_state.m_board[xyToPoint(j, i)]];
      }
      ss << endl;
    }
    s = ss.str();
  }

  void Board::print() const {
    std::string str;
    createShowString(str);
    cout << str;
  }

  void Board::printToErr() const {
    std::string str;
    createShowString(str);
    cerr << str;
  }

  void Board::printInvertedBoardToErr() const {
    string str;
    createInvertedShowString(str);
    cerr << str;
  }

  void Board::printAllBlocks() const {
    for (size_t z=0; z<m_state.m_belongBlocks.size(); z++) {
      if (m_state.m_belongBlocks.at(z)) {
        cerr << pointToXY(z).first << "," << pointToXY(z).second << ":" << m_state.m_belongBlocks.at(z) << endl;
      }
    }
  }
}
