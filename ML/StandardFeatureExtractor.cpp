#include "precomp.h"
#include "ML/StandardFeatureExtractor.h"
#include "utility/utility.h"

#include <fstream>

using namespace std;
using namespace Common;
using namespace Go;

namespace ML {

  StandardFeatureExtractor::StandardFeatureExtractor()
    : m_indexToHashValue()
    , m_hashValueToIndex()
    , m_patternExtractorsForEachSize(20)
      //, m_patternExtractor_boardsize9(9)
      //, m_patternExtractor_boardsize19(19)
  {
    for (size_t i=0; i<3; i++) {
      m_patternExtractorsForEachSize[i] = NULL;
    }
    for (size_t i=3; i<=19; i++) {
      m_patternExtractorsForEachSize[i] = new PatternExtractor_3x3(i);
    }
  }

  StandardFeatureExtractor::StandardFeatureExtractor(const StandardFeatureExtractor &rhs)
    : m_indexToHashValue(rhs.m_indexToHashValue)
    , m_hashValueToIndex(rhs.m_hashValueToIndex)
    , m_patternExtractorsForEachSize(20)
      //, m_patternExtractor_boardsize9(9)
      //, m_patternExtractor_boardsize19(19)
  {
    for (size_t i=0; i<3; i++) {
      m_patternExtractorsForEachSize[i] = NULL;
    }
    for (size_t i=3; i<=19; i++) {
      m_patternExtractorsForEachSize[i] = new PatternExtractor_3x3(i);
    }
  }

  StandardFeatureExtractor::~StandardFeatureExtractor() {
    for (size_t i=0; i<m_patternExtractorsForEachSize.size(); i++) {
      delete m_patternExtractorsForEachSize[i];
    }
  }

  // void StandardFeatureExtractor::extractFromStateAndAction(SparseVector &sparse_result, const Go::Board *state, Common::Point action, Common::Color turn) const {
  //   // sparse representation
  //   assert(state);
  //   sparse_result.clear();

  //   extractStaticFeaturesToSparseVector(sparse_result, state, action, turn);
  //   PatternExtractor_3x3 *extractor = m_patternExtractorsForEachSize[state->getSize()];
  //   extractPatternFeaturesToSparseVector(sparse_result, *extractor, state, action, turn);
  // }


  // void StandardFeatureExtractor::extractStaticFeaturesToSparseVector(SparseVector &result, const Board *state, Point action, Color turn)  const {
  //   if (action == PASS) {
  //     result.add(STATIC_FEATURE_CURRENT_MOVE_IS_PASS);
  //   }

  //   if (isSelfAtari(state, action, turn)) result.add(STATIC_FEATURE_SELF_ATARI);

  //   StaticFeatureIndex index = checkAtariFeatures(state, action, turn);
  //   if (index != STATIC_FEATURE_INVALID) result.add(index);

  //   if (isCapture(state, action, turn)) {
  //     result.add(STATIC_FEATURE_CAPTURE);
  //     if (isRecapture(state, action, turn)) result.add(STATIC_FEATURE_CAPTURE_RE_CAPTURE);
  //   }

  //   Point lastMove = state->getLastMove();
  //   if (lastMove != POINT_NULL) {
  //     if (lastMove == PASS) {
  //       //result.add(STATIC_FEATURE_PREVIOUS_MOVE_IS_PASS);
  //     } else {
  //       // check STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4
  //       std::pair<int,int> lastXY(state->pointToXY(lastMove));
  //       std::pair<int,int> currentXY(state->pointToXY(action));
  //       std::pair<int,int> diffXY(abs(lastXY.first-currentXY.first), abs(lastXY.second-currentXY.second));
        
  //       if (diffXY.first <= 1 && diffXY.second <= 1) {
  //         result.add(STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4);
  //       }
  //     }
  //   }

  // }

  // extract for all moves of the given board
  void StandardFeatureExtractor::extractFromStateForAllMoves(const Go::Board *board, Common::Color turn, Common::SparseVector *featureTableToExtract, Common::SparseVector &forPassFeature, Common::PointSet &extractedLegalMoves, bool clearOldFeatures) const {
    assert (board);
    assert (featureTableToExtract);

    if (clearOldFeatures) {
      for (int y=0; y<board->getSize(); y++) {
        Point p = board->xyToPoint(0,y);
        for (int x=0; x<board->getSize(); x++, p++) {
          featureTableToExtract[p].clear();
        }
      }
      //featureTableToExtract[0].clear();
    }
    extractedLegalMoves.clear();
    forPassFeature.clear();
    forPassFeature.add(STATIC_FEATURE_CURRENT_MOVE_IS_PASS);

    Color enemy = Board::flipColor(turn);

    // update pattern features
    for (int y=0; y<board->getSize(); y++) {
      for (int x=0; x<board->getSize(); x++) {
        Point p = board->xyToPoint(x,y);
        if (board->isColor(p, FREE) &&
            (board->getNeighborEmptyCount(p)>=1 || board->checkLegalHand(p, turn, enemy) == Board::PUT_LEGAL)) {
          updatePatternFeature(featureTableToExtract[p], board, p, turn);
          extractedLegalMoves.insert(p,p);
        }
      }
    }
    //updatePatternFeature(featureTableToExtract[0], board, PASS, turn);

    // update static features
    PointSet updatedMoves;
    updateStaticFeaturesForAllMovesWithoutClearOldFeatures(board, turn, extractedLegalMoves, featureTableToExtract, updatedMoves);
  }

  // update static features for the moves of given board
  void StandardFeatureExtractor::updateStaticFeaturesForAllMovesWithoutClearOldFeatures(const Go::Board *state, Common::Color player, const Common::PointSet &allLegalMoves, Common::SparseVector *featureTable, Common::PointSet &addMoves) const {
    initNeighborBlockSetCacheTable(state, m_neighborBlockSetCacheMy);
    initNeighborBlockSetCacheTable(state, m_neighborBlockSetCacheEnemy);

    const Board::MoveChangeEntry *lastMove = state->getLastHistory();
    if (lastMove != NULL) {
      _internal_updateStaticFeaturesForPreviousMoves(state, lastMove, player, featureTable, addMoves, m_neighborBlockSetCacheMy, m_neighborBlockSetCacheEnemy);
    }
    const Board::MoveChangeEntry *secondLastMove = state->getHistory(state->getHistoryCount()-2);
    if (secondLastMove != NULL) {
      enumerateMovesAndAddFeatureForDistanceFromSecondLastMoveIsLessThan4(addMoves, state, secondLastMove, featureTable, player);
    }
    
    enumerateMovesAndAddFeaturesForFeatures_Capture_SelfAtari_Atari(addMoves, state, allLegalMoves, featureTable, player, m_neighborBlockSetCacheMy, m_neighborBlockSetCacheEnemy);
  }

  // internal method for updateStaticFeaturesForAllMovesWithoutClearOldFeatures
  void StandardFeatureExtractor::_internal_updateStaticFeaturesForPreviousMoves(const Go::Board *board, const Go::Board::MoveChangeEntry *move, Common::Color player, Common::SparseVector *featureTable, Common::PointSet &addTo, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) {
    assert(board); assert(move);
    // check around
    if (move->m_putPos == PASS) return;

    enumerateMovesAndAddFeatureForDistanceFromLastMoveIsLessThan4(addTo, board, move, featureTable, player);

    if (move->m_killedBlocks[0] != NULL) {
      // check Re-capture
      const BlockPtr block = board->getBelongBlock(move->m_putPos);
      assert (block != NULL);
      assert (block->getColor() == Board::flipColor(player));
      if (block->getLibertyCount() == 1) {
        Point lib = block->getLiberties()[0];
        if (isRecapture(board, lib, player, blockSetCacheTableMy, blockSetCacheTableEnemy)) {
          addTo.insert(lib,lib);
          featureTable[lib].add(StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_RE_CAPTURE);
        }
      }
    }


    PointSet v, extensions;
    enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(board, player, v, extensions, blockSetCacheTableMy, blockSetCacheTableEnemy);
    for (size_t i=0; i<v.size(); i++) {
      assert (board->isColor(v[i], FREE));
      if (board->getNeighborEmptyCount(v[i]) >= 1 || board->checkLegalHand(v[i], player, Board::flipColor(player)) == Board::PUT_LEGAL) {
        addTo.insert(v[i], v[i]);
        featureTable[v[i]].add(StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_STRING_TO_NEW_ATARI);
      }
    }

    // moves in extensions cannot be illegal moves
    for (size_t i=0; i<extensions.size(); i++) {
      assert (board->isColor(extensions[i], FREE));
      assert (board->checkLegalHand(extensions[i], player, Board::flipColor(player)) == Board::PUT_LEGAL);
      addTo.insert(extensions[i], extensions[i]);
      featureTable[extensions[i]].add(StandardFeatureExtractor::STATIC_FEATURE_EXTENSION_NEW_ATARI);
    }
  }

  // check self-atari
  bool StandardFeatureExtractor::isSelfAtari(const Board *state, Common::Point action, Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) {
    int emptyCount = state->getNeighborEmptyCount(action);
    Color enemy = Board::flipColor(turn);
    if (emptyCount >= 2) return false;

    if (state->getNeighborCount(action, turn) == 0) return false;
    
    int captureCount = 0;
    Board::NeighborBlockSet &blocksetMy = getBlockSet(state, action, turn, blockSetCacheTableMy);
    Board::NeighborBlockSet &blocksetEn = getBlockSet(state, action, enemy, blockSetCacheTableEnemy);
    //Board::NeighborBlockSet blocksetMy;
    //state->getNeighborBlocks(action, turn, blocksetMy);
    //Board::NeighborBlockSet blocksetEn;
    //state->getNeighborBlocks(action, enemy, blocksetEn);
    
    //Board::NeighborBlockSet block
    PointSet liberties;
    //state->getNeighborBlocks(action, blockset);
    for (size_t i=0; i<blocksetEn.size(); i++) {
      BlockPtr block = blocksetEn[i];
      //if (block->getColor() == enemy) {
        if (block->getLibertyCount() == 1) {
          captureCount += block->getStoneCount();
          if (captureCount >= 2) return false;
        }
        //}
    }
    for (size_t i=0; i<blocksetMy.size(); i++) {
      BlockPtr block = blocksetMy[i];
      if (block->getLibertyCount() >= 3 ||
          (captureCount > 0 && block->getLibertyCount() >= 2)) return false;
      for (PointSet::ListConstIterator it=block->getLiberties().begin(); it!=block->getLiberties().end(); it++) {
        liberties.insert(*it,*it);
      }
      if (liberties.size() > 2) return false;
    }
    if (captureCount == 1) {
      return liberties.size() == 1;
    }

    return true;
  }

  // check atari
  StandardFeatureExtractor::StaticFeatureIndex StandardFeatureExtractor::checkAtariFeatures(const Board *state, Point action, Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) {
    Board::NeighborBlockSet &blockset = getBlockSet(state, action, Board::flipColor(turn), blockSetCacheTableEnemy);
    //Board::NeighborBlockSet blockset;
    //state->getNeighborBlocks(action, Board::flipColor(turn), blockset);
    for (size_t i=0; i<blockset.size(); i++) {
      BlockPtr block = blockset[i];
      if (block->getLibertyCount() == 2) {
        return (state->getKou() == PASS || state->getKou() == POINT_NULL) ?
          STATIC_FEATURE_ATARI_OTHER_SITUATIONS : STATIC_FEATURE_ATARI_WHEN_THERE_IS_A_KOU;
      }
    }
    return STATIC_FEATURE_INVALID;
  }

  void StandardFeatureExtractor::enumerateMovesOfPatternChanged(const Board *state, const Board::MoveChangeEntry *checkHistory, Color turn, PointSet &moves, bool clearSet) {
    if (clearSet) {
      moves.clear();
    }

    Point move = checkHistory->m_putPos;
    if (move == PASS || move == POINT_NULL) return;

    // around last move
    for (int i=0; i<8; i++) {
      Point p = move + state->getEightDirections()[i];
      if (!state->isColor(p, FREE)) continue;
      moves.insert(p,p);
    }
    

    // around killed blocks
    for (int i=0; i<4 && checkHistory->m_killedBlocks[i]; i++) {
      BlockPtr block = checkHistory->m_killedBlocks[i];
      for (size_t j=0; j<block->getStoneCount(); j++) {
        Point stoneMove = block->getStones()[j];
        for (int k=0; k<8; k++) {
          Point p = stoneMove + state->getEightDirections()[k];
          if (!state->isColor(p,FREE)) continue;
          moves.insert(p,p);
        }
      }
    }
  }

  // check capture string to new atari string
  //  and check for Extension (without ladder knowl)
  void StandardFeatureExtractor::enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(const Board *state, Color turn, PointSet &moves, PointSet &extensionMoves, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) {
    Point lastMove = state->getLastMove();

    //moves.clear();

    if (lastMove == POINT_NULL || lastMove == PASS) {
      return;
    }

    Board::NeighborBlockSet &myBlocks = getBlockSet(state, lastMove, turn, blockSetCacheTableMy);
    //Board::NeighborBlockSet myBlocks;// = getBlockSet(state, lastMove, turn, *blockSetCacheTableMy);
    //state->getNeighborBlocks(lastMove, turn, myBlocks);

    if (myBlocks.size() == 0) return;

    // check atari my blocks
    int atari_block_count = 0;
    for (size_t i=0; i<myBlocks.size(); i++) {
      BlockPtr block = myBlocks[i];
      if (block->getLibertyCount() == 1) {
        atari_block_count++;
        // atari block
        // check around of atari stones
        for (size_t j=0; j<block->getStoneCount(); j++) {
          Point p = block->getStones()[j];
          for (int k=0; k<4; k++) {
            Point pn = p + state->getFourDirections()[k];
            BlockPtr enemy = state->getBelongBlock(pn);
            if (!enemy || enemy->getColor() == turn) continue;
            if (enemy->getLibertyCount() == 1) {
              moves.insert(enemy->getLiberties()[0],enemy->getLiberties()[0]);
            }
          }
        }
        // check liberty of atari block
        Point lib = block->getLiberties()[0];
        int freeCount = state->getNeighborEmptyCount(lib);
        Board::NeighborBlockSet &libNeiBlocks = getBlockSet(state, lib, turn, blockSetCacheTableMy);
        //Board::NeighborBlockSet libNeiBlocks;// = getBlockSet(state, lib, turn, *blockSetCacheTableMy);
        //state->getNeighborBlocks(lib, turn, libNeiBlocks);
        for (size_t j=0; j<libNeiBlocks.size(); j++) {
          freeCount += libNeiBlocks[j]->getLibertyCount()-1;
          if (freeCount >= 2) {
            extensionMoves.insert(lib,lib);
            break;
          }
        }
      }
    }

    if (atari_block_count == 0) return;

    {
      // around last move
      BlockPtr block = state->getBelongBlock(lastMove);
      if (block->getLibertyCount() == 1) {
        Point p = block->getLiberties()[0];
        if (state->getKou() != p) {
          moves.insert(p,p);
        }
      }
    }
  }

  void StandardFeatureExtractor::enumerateMovesAndAddFeatureForDistanceFromLastMoveIsLessThan4(Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *lastMove, Common::SparseVector *featureTable, Common::Color player, bool doClearAddToSet) {
    _internal_enumerateMovesAndAddFeatureForDistanceFeature(STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4, addTo, board, lastMove, featureTable, player, doClearAddToSet);
  }

  void StandardFeatureExtractor::enumerateMovesAndAddFeatureForDistanceFromSecondLastMoveIsLessThan4(Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *secondLast, Common::SparseVector *featureTable, Common::Color player, bool doClearAddToSet) {
    _internal_enumerateMovesAndAddFeatureForDistanceFeature(STATIC_FEATURE_DIST_FROM_BEFORE_PREVIOUS_IS_LESS_THAN_4, addTo, board, secondLast, featureTable, player, doClearAddToSet);
  }

  void StandardFeatureExtractor::_internal_enumerateMovesAndAddFeatureForDistanceFeature(StaticFeatureIndex index, Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *move, Common::SparseVector *featureTable, Common::Color player, bool doClearAddToSet) {
    if (doClearAddToSet) {
      addTo.clear();
    }

    assert (move);

    if (move->m_putPos == PASS) return;

    for (int i=0; i<8; i++) {
      Point p = board->getEightDirections()[i] + move->m_putPos;
      if (board->isColor(p, FREE)) {
        // check DIST FROM PREV is LESS THAN 4
        if (board->getNeighborEmptyCount(p) >= 1 || board->checkLegalHand(p, player, Board::flipColor(player)) == Board::PUT_LEGAL) {
          addTo.insert(p, p);
          featureTable[p].add(index);
        }
      }
    }
  }

  void StandardFeatureExtractor::enumerateMovesAndAddFeaturesForFeatures_Capture_SelfAtari_Atari(Common::PointSet &addTo, const Go::Board *board, const Common::PointSet &allLegalMoves, Common::SparseVector *featureTable, Common::Color player, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy, bool doClearAddToSet) {
    if (doClearAddToSet) {
      addTo.clear();
    }

    Color enemy = Board::flipColor(player);

    for (size_t i=0; i<allLegalMoves.size(); i++) {
      Point p = allLegalMoves[i];

      assert (board->isColor(p, FREE));
      assert (board->getNeighborEmptyCount(p) >= 1 || board->checkLegalHand(p, player, enemy) == Board::PUT_LEGAL);

      if (isCapture(board, p, player, blockSetCacheTableMy, blockSetCacheTableEnemy)) {
        featureTable[p].add(STATIC_FEATURE_CAPTURE);
        addTo.insert(p, p);
      }
      if (isSelfAtari(board, p, player, blockSetCacheTableMy, blockSetCacheTableEnemy)) {
        featureTable[p].add(STATIC_FEATURE_SELF_ATARI);
        addTo.insert(p,p);
      }

      StaticFeatureIndex index = checkAtariFeatures(board, p, player, blockSetCacheTableMy, blockSetCacheTableEnemy);
      if (index != STATIC_FEATURE_INVALID) {
        featureTable[p].add(index);
        addTo.insert(p,p);
      }
    }

  }

  // check normal capture
  bool StandardFeatureExtractor::isCapture(const Go::Board *state, Common::Point action, Common::Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) {
    // this feature is already true whenever Re-capture feature or capture next to new-atari feature is true
    if (action == PASS) return false;

    if (action == state->getKou()) return false;
    
    Board::NeighborBlockSet &blocks = getBlockSet(state, action, Board::flipColor(turn), blockSetCacheTableEnemy);
    //Board::NeighborBlockSet blocks;
    //state->getNeighborBlocks(action, Board::flipColor(turn), blocks);
    if (blocks.size() == 0) return false;

    for (size_t i=0; i<blocks.size(); i++) {
      if (blocks[i]->getLibertyCount() == 1) {
        return true;
      }
    }
    return false;
  }

  // check re-capture feature
  bool StandardFeatureExtractor::isRecapture(const Board *state, Point action, Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) {
    const Board::MoveChangeEntry *entry = state->getLastHistory();

    if (entry == NULL) return false; // no history

    if (!state->isColor(entry->m_putPos, Board::flipColor(turn))) return false; // impossible pattern (last move is not enemy's)

    if (entry->m_killedBlocks[0] == NULL) return false; // no capture

    // isCapture does: kou check, around check, liberty count of neighbor check
    if (!isCapture(state, action, turn, blockSetCacheTableMy, blockSetCacheTableEnemy)) return false; // cannot capture

    BlockPtr block = state->getBelongBlock(entry->m_putPos);
    if (block->getLibertyCount() != 1) return false; // cannot capture the last move
    
    Point candidateMove = block->getLiberties()[0];
    if (candidateMove != action) return false; // action is not the move

    return true;
  }

  Go::Board::NeighborBlockSet &StandardFeatureExtractor::getBlockSet(const Go::Board *board, Common::Point p, Common::Color c, NeighborBlockSetCache *cacheTable) {
    if (cacheTable[p].needToRefresh) {
      board->getNeighborBlocks(p, c, cacheTable[p].blockSet);
      cacheTable[p].needToRefresh = false;
    }
    return cacheTable[p].blockSet;
  }

  void StandardFeatureExtractor::initNeighborBlockSetCacheTable(const Go::Board *board, NeighborBlockSetCache *table) {
    for (int y=0; y<board->getSize(); y++) {
      Point p = board->xyToPoint(0, y);
      for (int x=0; x<board->getSize(); x++, p++) {
        table[p].needToRefresh = true;
      }
    }
  }

  //--------------------------------------
  // for pattern features method
  //-------------------------------------

  void StandardFeatureExtractor::updatePatternFeature(Common::SparseVector &old_vector, const Go::Board *state, Common::Point action, Common::Color turn) const {
    assert (action != POINT_NULL && action != PASS);
    clearPatternFeatures(old_vector);
    PatternExtractor_3x3 &pattern_extractor = *m_patternExtractorsForEachSize[state->getSize()];
    PATTERN_3x3_HASH_VALUE value = pattern_extractor.encode(*state, action, turn == WHITE);
    int index = getFeatureIndexOfPattern(value);
    if (index != -1) {
      old_vector.add(index);
    }
  }

  // void StandardFeatureExtractor::updateStaticFeature(Common::SparseVector &old_vector, const Go::Board *state, Common::Point action, Common::Color turn) const {
  //   clearStaticFeatures(old_vector);
  //   extractStaticFeaturesToSparseVector(old_vector, state, action, turn);
  // }

  // Extracts pattern features
  void StandardFeatureExtractor::extractPatternFeaturesToSparseVector(SparseVector &result, const PatternExtractor_3x3 &pattern_extractor, const Board *state, Point action, Color turn) const {
    assert(action != POINT_NULL);
    if (action != PASS) {
      PATTERN_3x3_HASH_VALUE value = pattern_extractor.encode(*state, action, turn == WHITE);
      int index = getFeatureIndexOfPattern(value);
      if (index != -1) {
        result.add(index);
      }
    }
  }

  size_t StandardFeatureExtractor::getFeatureDimension() const {
    return static_cast<size_t>(STATIC_FEATURE_SIZE) + m_indexToHashValue.size();
  }

  void StandardFeatureExtractor::clearStaticFeatures(SparseVector &vec) {
    // assumption: at most one pattern feature is included
    int pattern_index = -1;
    for (int i=(signed)vec.size() - 1; i>=0; i--) {
      if (vec[i] >= STATIC_FEATURE_SIZE) {
#ifndef DEBUG
        pattern_index = vec[i];
        break;
#else
        assert (pattern_index == -1);
        pattern_index = vec[i];
#endif
      }
    }
    vec.clear();
    if (pattern_index != -1) vec.add(pattern_index);
  }
  void StandardFeatureExtractor::clearPatternFeatures(SparseVector &vec) {
    // assumption: at most one pattern feature is included
    int count = 0;
    for (int i=(signed)vec.size() - 1; i>=0; i--) {
      if (vec[i] >= STATIC_FEATURE_SIZE) {
        vec.erase(i);
#ifndef DEBUG
        return;
#else
        count++;
        assert (count <= 1);
#endif
      }
    }
  }

  // 1. Pre-process: Registering patterns as features
  //  TODO: 1.5 A method: get a pattern hash, then register it to the table and returns feature index
  int StandardFeatureExtractor::registerPatternAsFeature(PATTERN_3x3_HASH_VALUE pattern) {
    if (pattern == PatternExtractor_3x3::INVALID_HASH_VALUE) return -1;

    HashIndexTable::const_iterator it = m_hashValueToIndex.find(pattern);
    if (it == m_hashValueToIndex.end()) {
      int newindex = static_cast<int>(m_indexToHashValue.size());
      m_indexToHashValue.push_back(pattern);
      m_hashValueToIndex[pattern] = newindex;
      return newindex;
    } else {
      return it->second;
    }
  }

  int StandardFeatureExtractor::getFeatureIndexOfPattern(PATTERN_3x3_HASH_VALUE pattern) const {
    HashIndexTable::const_iterator it = m_hashValueToIndex.find(pattern);
    if (it != m_hashValueToIndex.end()) {
      return STATIC_FEATURE_SIZE + it->second;
    }
    return -1;
  }

  PATTERN_3x3_HASH_VALUE StandardFeatureExtractor::getPatternOfFeatureIndex(int index) const {
    if (index < STATIC_FEATURE_SIZE || static_cast<size_t>(index) >= STATIC_FEATURE_SIZE + m_indexToHashValue.size()) {
      return PatternExtractor_3x3::INVALID_HASH_VALUE;
    }
    return m_indexToHashValue[index-STATIC_FEATURE_SIZE];
  }

  bool StandardFeatureExtractor::readPatternsAndWeightsFromFile(const std::string &weight_file, std::vector<PATTERN_3x3_HASH_VALUE> &values, std::vector<shared_ptr<FeatureWeights> > &weights) {
    ifstream ifs(weight_file.c_str());

    if (!ifs) {
      cerr << "Failed to load pattern file: " << weight_file << endl;
      values.clear();
      weights.clear();
      return false;
    }
  
    string line;
    do {
      getline(ifs, line);
    } while (line.find("feature index to pattern hash") == string::npos);

    getline(ifs, line);

    vector<string> split_data;
    Common::split(line, split_data, ",");

    size_t featureSize = split_data.size();

    values.reserve(split_data.size());
    for (size_t i=0; i<split_data.size(); i++) {
      if (split_data[i] != "-1") {
        PATTERN_3x3_HASH_VALUE value = static_cast<PATTERN_3x3_HASH_VALUE>(strtol(split_data[i].c_str(), NULL, 16));
        values.push_back(value);
      }
    }

    // read weights
    while (getline(ifs,line)) {
      if (line.size() == 0) continue;
      if (line[0] == '(') line = line.substr(1);
      if (line[line.size()-1] == ')') line = line.substr(0, line.size()-1);
      Common::split(line, split_data, ",");

      // stochastic の resultは、 ",)"みたいなのがある
      assert(split_data.size() == featureSize);
      shared_ptr<FeatureWeights> weight(new FeatureWeights(featureSize));

      for (size_t i=0; i<split_data.size(); i++) {
        string value_str = split_data[i];
        if (value_str.size() == 0) {
          cerr << "fatal error in reading weights" << endl;
          values.clear();
          weights.clear();
          return false;
        }
        if (value_str[0] == '(') value_str = value_str.substr(1);
        if (value_str[value_str.size()-1] == ')') value_str = value_str.substr(0, value_str.size()-1);
        if (value_str.find("nan") != string::npos) {
          weight.reset();
          break;
        }
        (*weight)[i] = atof(value_str.c_str());
      }
      if (weight != NULL) {
        weights.push_back(weight);
      }
    }
    return true;
  }

}
