
#pragma once

#include "precomp.h"

#include <vector>
#include <stack>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <bitset>
#include "common.h"

#include "utility/NeighborSet.h"
#include "BoardHash.h"

namespace Go {  

  class Block;

  class Board {
  public:
    
    class BoardState {
      friend class Board;
      BoardState(const BoardState &rhs){}
      BoardState &operator =(const BoardState &rhs){return *this;}

      inline Common::Point rawXYtoPoint(int x, int y) const {
        return x + y*m_size;
      }

    public:
      typedef std::bitset<MAX_BOARD_SIZE> MoveFlagSet;
      Board *m_boardOrig;

    public:
      int m_size;
      Common::Point m_putPos;
      Common::Color m_putColor;
      Common::Point m_kouPos;
      int m_capturedCount[4];
      MoveFlagSet m_usedList;
      BoardHash m_hash;
      std::vector<int> m_neighborCount[4];
      std::vector<int> m_neighborEmptyCount;
      std::vector<int> m_neighborWallCount;
      std::vector<BlockPtr> m_belongBlocks;
      bool m_isNewPosition; // if this is true, any repetition cannot occur
      Common::Color *m_board;
      bool *m_isFirst;

      explicit BoardState(Board *board);
      explicit BoardState(int size, Board *board);
      ~BoardState();
      typedef std::unordered_map<Block *, BlockPtr> BlocksMapping;
      void copyFrom(const BoardState &rhs, BlocksMapping &mapping);
      void clear();

      inline Common::Point xyToPoint(int x, int y) const {
        return x+1 + (y+1)*m_size;
      }
      inline std::pair<int,int> pointToXY(Common::Point z) const {
        return std::pair<int,int>( z%m_size-1, z/m_size-1 );
      }
    };

  public:
    class MoveChangeEntry {
    public:
      Common::Point m_putPos;
      Common::Color m_putColor;
      bool m_isFirst;
      bool m_isNewPosition; // if this is true, any repetition cannot occur
      Common::Point m_kouPos;
      Common::Color m_kouColor;
      BoardHash m_hash;
      BlockPtr m_addedBlock; // this cannot be null
      Common::Point m_addedLiberties[4];
      BlockPtr m_mergedBlocks[4]; // merged blocks expect for m_addedBlock
      BlockPtr m_killedBlocks[4];
      // shared_ptr<Block> m_suicideBlock;
      MoveChangeEntry();
      ~MoveChangeEntry();
      static MoveChangeEntry *createNewEntry();
      static void releaseEntry(MoveChangeEntry *entry);
      static void copyVector(std::vector<MoveChangeEntry *> &copy_to, std::vector<MoveChangeEntry *> &copy_from, Board::BoardState::BlocksMapping &mapping);
    private:
      MoveChangeEntry(const MoveChangeEntry &entry){}
      MoveChangeEntry &operator =(const MoveChangeEntry &entry){return *this;}

    private:
      MoveChangeEntry &copyFromWithMapping(const MoveChangeEntry &from, Board::BoardState::BlocksMapping &mapping);
    };

  private:
    // private method for MoveChangeEntry
    static void releaseMoveChangeEntryVector(std::vector<MoveChangeEntry *> &history);

  public:
    explicit Board(int size, Common::Color *init_array = NULL);
    ~Board();
    //void copyFrom(const Board &rhs);

    void clear();
    
    void print() const;
    void printToErr() const;
    void printInvertedBoardToErr() const;
    void createShowString(std::string &str) const;
    void createInvertedShowString(std::string &s) const;
    inline int getSize() const {return m_originalSize;}

    enum PutType {
      PUT_LEGAL, PUT_SUICIDE, PUT_KOU, PUT_EYE, PUT_NOT_FREE
    };
    PutType put(int x, int y, Common::Color color, bool withoutLegalCheck = false)
    {
      if (x==Common::PASS || y==Common::PASS) return put(Common::PASS, color);
      else return put(xyToPoint(x,y), color, withoutLegalCheck);
    }
    PutType put(Common::Point z, Common::Color color, bool withoutLegalCheck = false);
    PutType checkLegalHand(Common::Point z, Common::Color color) const {
      return checkLegalHand(z, color, flipColor(color));
    }
    PutType checkLegalHand(Common::Point z, Common::Color color, Common::Color enemy_color) const;
    void enumerateFreeMoves(Common::Point *point_array, int &point_count, Common::Color turn) const;

    bool undo();
    void takeSnapshot();
    void restoreStateFromSnapshot(); // This method BREAK MOVE_CHANGE_HISTORY !!! 

    double countScore(Common::Color turn_color);
    void countScoreDetail(double &black, double &white);
    int getCapturedCount(Common::Color turn) const;

    //typedef std::unordered_set<std::shared_ptr<Block>, blockptrhash> Common::NeighborBlockSet;
    typedef Common::NeighborSet<BlockPtr> NeighborBlockSet;

    void getNeighborBlocks(Common::Point p, NeighborBlockSet &gotBlocks) const;
    void getNeighborBlocks(Common::Point p, Common::Color color, NeighborBlockSet &gotBlocks) const;
    inline int getNeighborEmptyCount(Common::Point posZ) const {
      return m_state.m_neighborEmptyCount[posZ];
    }
    inline int getNeighborCount(Common::Point posZ, Common::Color color) const {
      assert(color == Common::BLACK || color == Common::WHITE);
      return m_state.m_neighborCount[color][posZ];//m_state.m_neighborCount.find(color)->second[posZ];
    }

    bool isRepeatedPosition() const;

    const BlockPtr getBelongBlock(int x, int y) const {
      return getBelongBlock(xyToPoint(x,y));
    }
    const BlockPtr getBelongBlock(Common::Point z) const {
#ifdef DEBUG
      return m_state.m_belongBlocks.at(z);
#else
      return m_state.m_belongBlocks[z];
#endif
    }

    inline Common::Color getStone(int x, int y) const {
      return m_state.m_board[xyToPoint(x, y)];
    }
    inline Common::Color getStone(Common::Point z) const {
      return m_state.m_board[z];
    }
    inline Common::Color getToPlay() const {
      return flipColor(getLastHistory()->m_putColor);
    }
    inline Common::Point getKou() const {
      return m_state.m_kouPos;
    }
    inline Common::Point xyToPoint(int x, int y) const {
      return m_state.xyToPoint(x,y);
    }
    inline std::pair<int,int> pointToXY(Common::Point z) const {
      return m_state.pointToXY(z);
    }
    inline bool isColor(int x, int y, Common::Color color) const {
      return isColor(xyToPoint(x, y), color);
    }
    inline bool isFirst(int x, int y) const {
      return isFirst(xyToPoint(x,y));
    }
    inline bool isFirst(Common::Point p) const {
      return m_state.m_isFirst[p];
    }

    inline static Common::Color flipColor(Common::Color color) {
      return Common::WALL-color;
    }

    void printAllBlocks() const;

    inline bool isColor(Common::Point z, Common::Color color) const {
      return m_state.m_board[z] == color;
    }
    bool isAdjacentBlock(Common::Point p, const Block* b) const;

    inline size_t getHistoryCount() const {
      return m_changeHistory.size();
    }
    inline Common::Point getMoveHistory(size_t historyIndex) const {
      if (historyIndex >= m_changeHistory.size()) return Common::POINT_NULL;
      return m_changeHistory[historyIndex]->m_putPos;
    }
    inline const MoveChangeEntry *getHistory(size_t historyIndex) const {
      if (historyIndex >= m_changeHistory.size()) return NULL;
      return m_changeHistory[historyIndex];
    }
    inline const MoveChangeEntry *getLastHistory() const {
      return getHistory(m_changeHistory.size()-1);
    }
    inline Common::Point getLastMove() const {
      if (getHistoryCount() == 0) return Common::POINT_NULL;
      return m_changeHistory[m_changeHistory.size()-1]->m_putPos;
    }

    inline const BoardHash &getHash() const {
      return m_state.m_hash;
    }

  private:
    void initFromArray(const Common::Color *array);

    inline Common::Point rawXYtoPoint(int x, int y) const {
      return m_state.rawXYtoPoint(x,y);
    }

    bool isFalseEye(Common::Point z, Common::Color c) const;
    bool isSuicide(Common::Point p, Common::Color c) const;


    void addStone(Common::Point p, Common::Color c);
    void addLibertiesToAdjacentBlocks(Common::Point p);
    void addLibertiesToAdjacentBlocks(Common::Point p, Common::Color c);
    void removeStone(Common::Point p);
    void removeLibertiesToAdjacentBlocks(Common::Point p);
    void removeLibertiesToAdjacentBlocks(Common::Point p, Common::Color c);

    void createNewSingleBlock(Common::Point posZ, Common::Color color, MoveChangeEntry &changesEntry);
    void mergeTwoBlock(BlockPtr mergeTo, const BlockPtr& mergeFrom);
    void updateBlockAfterAddStone(Common::Point posZ, Common::Color color, MoveChangeEntry &changesEntry, NeighborBlockSet &neighbors);
    void addStoneToBlockAndUpdate(Common::Point posZ, Common::Color color, BlockPtr &block, MoveChangeEntry &changesEntry);
    void removeBlock(BlockPtr block);
    void removeLibertyAndKill(Common::Point posZ, Common::Color enemy_color, MoveChangeEntry &changesEntry, NeighborBlockSet &ownNeighborBuffer);

    void executePutAndUpdate(MoveChangeEntry &entry, Common::Point z, Common::Color color, Common::Color enemy_color);// __attribute__((noinline));

    MoveChangeEntry& createNewHistoryEntry();// __attribute__((noinline));
    void saveState(MoveChangeEntry &entry);
    void restoreState(const MoveChangeEntry &entry);
    void restoreKill(BlockPtr block, Common::Color c);
    void updateBlockAfterUndoChanges(const MoveChangeEntry &entry);

    void checkConsistency() const;

  public:
    inline const Common::Point* getFourDirections() const {return s_dirZ;}
    inline const Common::Point* getEightDirections() const {return s_allDirs;}

  private:

    /* int m_size;   // '2' is plused to original size (if 9 is passed to constructor, m_size is 11) */
    /*  // original size */
    /* Common::Point m_kouPos; */
    /* std::map<Color, int> m_hamaCount; */
    /* std::unordered_map<Color, std::vector<int> > m_neighborCount; */
    /* std::vector<int> m_neighborEmptyCount; */
    /* std::vector<int> m_neighborWallCount; */
    /* std::vector<std::shared_ptr<Block> > m_belongBlocks; */
    /*  */
    /* Common::Color *m_board; */

    int m_originalSize;
    std::vector<MoveChangeEntry *> m_changeHistory, m_snapshotHistory;
    BoardState m_state, m_snapshotState;

  private:
    Common::Point s_dirZ[4];

    Common::Point s_allDirs[8];
    Common::Point DIR_LEFT;
    Common::Point DIR_RIGHT;
    Common::Point DIR_UP;
    Common::Point DIR_DOWN;
    Common::Point DIR_RIGHTUP;
    Common::Point DIR_RIGHTDOWN;
    Common::Point DIR_LEFTUP;
    Common::Point DIR_LEFTDOWN ;


  };
}
