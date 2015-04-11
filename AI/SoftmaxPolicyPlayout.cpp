
#include "precomp.h"

#include "SoftmaxPolicyPlayout.h"
#include "utility/NeighborSet.h"
#include "utility/fmath.hpp"

#include <iostream>
#include <fstream>

using namespace Common;
using namespace ML;
using namespace Go;
using namespace std;

namespace AI {
  SoftmaxPolicyPlayout::SoftmaxPolicyPlayout(const Go::Board *initial_board, const std::vector<ML::PATTERN_3x3_HASH_VALUE> &pattern_list, const ML::FeatureWeights &weights, Common::MTRandom &rnd)
    : m_sequence()
      //, m_expFeatureWeights(weights.size())
      //, m_invertedExpFeatureWeights(weights.size())
    , m_featureWeights(weights.size())
    , m_featureExtractor()
    , m_rnd(rnd)
    , m_toResetStaticFeaturesMovesBlack()
    , m_toResetStaticFeaturesMovesWhite()
  {
    for (int i=0; i<MAX_BOARD_SIZE; i++) {
      m_featureTableBlack[i].clear();
      m_featureTableWhite[i].clear();
    }
    m_sequence.reserve(MAX_BOARD_SIZE + 10000);
    std::vector<ML::PATTERN_3x3_HASH_VALUE>::const_iterator it,end = pattern_list.end();
    for (it = pattern_list.begin(); it!=end; it++) {
      m_featureExtractor.registerPatternAsFeature(*it);
    }
    setFeatureWeights(weights);
  }

  SoftmaxPolicyPlayout::SoftmaxPolicyPlayout(const std::string &learnedFeatureWeightFile, Common::MTRandom &rnd)
    : m_sequence()
      //, m_expFeatureWeights(0)
    , m_featureExtractor()
    , m_rnd(rnd)
    , m_toResetStaticFeaturesMovesBlack()
    , m_toResetStaticFeaturesMovesWhite()
  {
    for (int i=0; i<MAX_BOARD_SIZE; i++) {
      m_featureTableBlack[i].clear();
      m_featureTableWhite[i].clear();
    }
    m_sequence.reserve(MAX_BOARD_SIZE + 10000);

    vector<PATTERN_3x3_HASH_VALUE> patterns;
    vector<shared_ptr<FeatureWeights> > weights;

    if (StandardFeatureExtractor::readPatternsAndWeightsFromFile(learnedFeatureWeightFile, patterns, weights)) {
      std::vector<ML::PATTERN_3x3_HASH_VALUE>::const_iterator it,end = patterns.end();
      for (it = patterns.begin(); it!=end; it++) {
        m_featureExtractor.registerPatternAsFeature(*it);
      }
      const FeatureWeights &targetWeights = *weights[weights.size()-1];
      setFeatureWeights(targetWeights);
    }
  }

  SoftmaxPolicyPlayout::~SoftmaxPolicyPlayout() {
  }

  void SoftmaxPolicyPlayout::setFeatureWeights(const ML::FeatureWeights &weights) {
    assert (m_featureExtractor.getFeatureDimension() == weights.size());
    m_featureWeights = weights;
    //m_expFeatureWeights.init(weights.size());
    //m_invertedExpFeatureWeights.init(weights.size());
    //for (size_t i=0; i<weights.size(); i++) {
    //m_expFeatureWeights[i] = exp(weights[i]);
      //   m_expFeatureWeights[i] = fmath::expd(weights[i]);
      //m_invertedExpFeatureWeights[i] = 1.0/m_expFeatureWeights[i];
    //}
    //cerr << m_expFeatureWeights.toString().substr(0,100) << endl;
  }

  void SoftmaxPolicyPlayout::addFeatureWeights(const ML::FeatureWeights &weights) {
    assert (m_featureExtractor.getFeatureDimension() == weights.size());
    m_featureWeights += weights;
    //m_expFeatureWeights.init(weights.size());
    // m_invertedExpFeatureWeights.init(weights.size());
    // for (size_t i=0; i<weights.size(); i++) {
    //   if (weights[i] != 0) {
    //     //m_expFeatureWeights[i] = fmath::expd(m_featureWeights[i]);
    //     //m_expFeatureWeights[i] = exp(m_featureWeights[i]);
    //   }
    //   //m_invertedExpFeatureWeights[i] = 1.0/m_expFeatureWeights[i];
    // }
    //cerr << m_expFeatureWeights.toString().substr(0,100) << endl;
  }

/*
  void SoftmaxPolicyPlayout::initProbabilities(const Go::Board *init_board) {
    //memset(m_probTableBlack, 0, sizeof(double)*MAX_BOARD_SIZE);
    //memset(m_probTableWhite, 0, sizeof(double)*MAX_BOARD_SIZE);

    //SparseVector extractedFeatures;
    Color turns[] = {BLACK, WHITE};
    double *tables[2] = {m_probTableBlack, m_probTableWhite};
    SparseVector *featureTables[2] = {m_featureTableBlack, m_featureTableWhite};

    for (int i=0; i<2 && tables[i] != NULL; i++) {
      Color myTurn = turns[i];

      initProbabilities(init_board, myTurn, tables[i], featureTables[i]);
    }
  }
*/
  void SoftmaxPolicyPlayout::initProbabilities(const Go::Board *init_board, Color myTurn, double *table, SparseVector *featureTable) {
    Color enemyTurn = Board::flipColor(myTurn);
    PointSet &updatedMoves = myTurn == BLACK ? m_toResetStaticFeaturesMovesBlack : m_toResetStaticFeaturesMovesWhite;
    updatedMoves.clear();

    for (int y=0; y<init_board->getSize(); y++) {
      for (int x=0; x<init_board->getSize(); x++) {
        featureTable[init_board->xyToPoint(x,y)].clear();
      }
    }

    PointSet legalMovesSet;

    // update pattern features
    for (int y=0; y<init_board->getSize(); y++) {
      for (int x=0; x<init_board->getSize(); x++) {
        Point p = init_board->xyToPoint(x,y);
        if (init_board->isColor(p, FREE) &&
            (init_board->getNeighborEmptyCount(p)>=1 || init_board->checkLegalHand(p, myTurn, enemyTurn) == Board::PUT_LEGAL)) {
          m_featureExtractor.updatePatternFeature(featureTable[p], init_board, p, myTurn);
          legalMovesSet.insert(p,p);
        } else {
          // prob is 0
          //featureTable[p].clear();
          if (init_board->isColor(p, FREE)) {
            m_featureExtractor.updatePatternFeature(featureTable[p], init_board, p, myTurn);
          }
          table[p] = 0;
        }
      }
    }

    // update static features
    m_featureExtractor.updateStaticFeaturesForAllMovesWithoutClearOldFeatures(init_board, myTurn, legalMovesSet, featureTable, updatedMoves);
    
    for (size_t i=0; i<legalMovesSet.size(); i++) {
      //table[legalMovesSet[i]] = m_expFeatureWeights.multiplyAll(featureTable[legalMovesSet[i]]);
      table[legalMovesSet[i]] = fmath::expd(m_featureWeights.dot(featureTable[legalMovesSet[i]]));
      //table[legalMovesSet[i]] = exp(m_featureWeights.dot(featureTable[legalMovesSet[i]]));
    }

  }

  // static size_t totalCandidateCounts = 0;
  // static size_t totalInsertedCounts = 0;
  // static size_t updatedCounts = 0;

  void SoftmaxPolicyPlayout::updateProbabilitiesBeforeAction(const Go::Board *board, Color player) {
    // check the last move
    size_t historySize = board->getHistoryCount();
    const Board::MoveChangeEntry *lastMove = board->getHistory(historySize-1);       // enemy's move
    const Board::MoveChangeEntry *secondLastMove = board->getHistory(historySize-2); // last my move
    const Board::MoveChangeEntry *thirdLastMove = board->getHistory(historySize-3);

    double *table = NULL;
    SparseVector *featureTable = NULL;
    if (player == BLACK) {
      table = m_probTableBlack;
      featureTable = m_featureTableBlack;
    } else {
      table = m_probTableWhite;
      featureTable = m_featureTableWhite;
    }

    if (lastMove && lastMove->m_putPos != PASS) table[lastMove->m_putPos] = 0;
    if (secondLastMove && secondLastMove->m_putPos != PASS) table[secondLastMove->m_putPos] = 0;

    PointSet updatedMoves;

    // reset static features which are set in the last time of player
    PointSet &previousSetMoves = player == BLACK ? m_toResetStaticFeaturesMovesBlack : m_toResetStaticFeaturesMovesWhite;
    for (size_t i=0; i<previousSetMoves.size(); i++) {
      //cerr << previousSetMoves[i] << ":" << featureTable[previousSetMoves[i]].toString() << endl;
      Point p = previousSetMoves[i];
      StandardFeatureExtractor::clearStaticFeatures(featureTable[p]);
      if (board->isColor(p, FREE) && 
          (board->getNeighborEmptyCount(p) >= 1 || board->checkLegalHand(p, player, Board::flipColor(player)) == Board::PUT_LEGAL)) {
        updatedMoves.insert(p,p);
      }
    }
    previousSetMoves.clear();

    // enumerate pattern update moves
    PointSet patternUpdateMoves;
    enumerateMovesOfPatternChanged(patternUpdateMoves, board, player, lastMove, secondLastMove, false);

    // enumerate moves that their status will change
    PointSet toBeLegal, toBeIllegal;
    PointSet legalMovesSet;
    toBeLegal.clear(); toBeIllegal.clear();
    enumerateToBeLegalAndIllegalMoves(board, table, player, toBeLegal, toBeIllegal, legalMovesSet, false);

    PointSet &staticFeatureUpdateMoves = previousSetMoves;

    if (lastMove == NULL) return; // no further old moves

    // update static features
    m_featureExtractor.updateStaticFeaturesForAllMovesWithoutClearOldFeatures(board, player, legalMovesSet, featureTable, staticFeatureUpdateMoves);

    // update patterns
    PointSet::ListConstIterator it, end = patternUpdateMoves.end();
    for (it = patternUpdateMoves.begin(); it!=end; it++) {
      SparseVector &targetFeatures = featureTable[*it];
      assert (board->isColor(*it, FREE));
      m_featureExtractor.updatePatternFeature(targetFeatures, board, *it, player);
      //if (board->checkLegalHand(*it, player, Board::flipColor(player)) == Board::PUT_LEGAL) {
      if ((!toBeIllegal.contains(*it) && table[*it] != 0) || toBeLegal.contains(*it)) {
        updatedMoves.insert(*it,*it);
      }
    }

    end = toBeLegal.end();
    for (it = toBeLegal.begin(); it!=end; it++) {
      updatedMoves.insert(*it,*it);
      // check capturing, atari features
    }

    end = staticFeatureUpdateMoves.end();
    for (it=staticFeatureUpdateMoves.begin(); it!=end; it++) {
      //table[*it] = m_expFeatureWeights.multiplyAll(featureTable[*it]);
      updatedMoves.insert(*it,*it);
    }

    end = updatedMoves.end();
    for (it=updatedMoves.begin(); it!=end; it++) {
      //table[*it] = m_expFeatureWeights.multiplyAll(featureTable[*it]);
      table[*it] = fmath::expd(m_featureWeights.dot(featureTable[*it]));
      //table[*it] = exp(m_featureWeights.dot(featureTable[*it]));
    }

    end = toBeIllegal.end();
    for (it = toBeIllegal.begin(); it!=end; it++) {
      table[*it] = 0;
      StandardFeatureExtractor::clearStaticFeatures(featureTable[*it]);
    }

#ifdef STRICT_CHECK
    for (int y=0; y<board->getSize(); y++) {
      for (int x=0; x<board->getSize(); x++) {
        Point p = board->xyToPoint(x,y);
        Board::PutType err = board->checkLegalHand(p, player, Board::flipColor(player));
        assert (!(err == Board::PUT_LEGAL && table[p] == 0) &&
                !(err != Board::PUT_LEGAL && table[p] != 0));
        if (err == Board::PUT_LEGAL) {
          double v = m_expFeatureWeights.multiplyAll(featureTable[p]);
          if (fabs(v-table[p]) >= 0.001) {
            cerr << "v = " << v << endl;
            cerr << "table[p] = " << table[p] << endl;
          }
          assert (fabs(v-table[p]) < 0.001);

          SparseVector vec;
          m_featureExtractor.extractFromStateAndAction(vec, board, p, player);
          // for (int i=0; i<vec.size(); i++) {
          //   if (vec[i] > 0 && vec[i] < StandardFeatureExtractor::STATIC_FEATURE_SIZE) {
          //     vec.erase(i);i--;
          //   }
          // }
          for (int i=0; i<featureTable[p].size(); i++) {
            if (featureTable[p][i] > 0 && featureTable[p][i] < 9) {
              featureTable[p].erase(i);i--;
            }
          }
          if (vec.size() != featureTable[p].size()) {
            // cerr << "updated moves:" << endl;
            // for (int i=0; i<staticFeatureUpdateMoves.size(); i++) {
            //   cerr << staticFeatureUpdateMoves[i] << ":" << featureTable[staticFeatureUpdateMoves[i]].toString() << endl;
            // }
            cerr << vec.toString() << endl;
            cerr << featureTable[p].toString() << endl;
            cerr << "point " << p << endl;
            cerr << "turn = " << player << endl;
            cerr << "last move = " << board->getLastMove() << endl;
            cerr << "second last move = " << secondLastMove->m_putPos << endl;
            board->printToErr();
            printProbabilityTableToErr(board);
          }
          assert (vec.size() == featureTable[p].size());
        }
      }
    }
#endif
  }

  // enumerate moves which are legal moves in the given table but acutually illegal, and vice-versa
  void SoftmaxPolicyPlayout::enumerateToBeLegalAndIllegalMoves(const Go::Board *board, double *probTable, Common::Color player, Common::PointSet &toBeLegal, Common::PointSet &toBeIllegal, Common::PointSet &allLegalMoves, bool clearResultSet) {
    if (clearResultSet) {
      toBeLegal.clear();
      toBeIllegal.clear();
      allLegalMoves.clear();
    }

    // scan all free moves
    for (int y=0; y<board->getSize(); y++) {
      for (int x=0; x<board->getSize(); x++) {
        Point p = board->xyToPoint(x,y);
        if (!board->isColor(p, FREE)) continue;

        if (probTable[p] == 0) {
          // p is illegal move in the probTable
          if (board->getNeighborEmptyCount(p) >= 1 || board->checkLegalHand(p, player, Board::flipColor(player)) == Board::PUT_LEGAL) {
            // actually legal
            toBeLegal.insert(p,p);
            allLegalMoves.insert(p,p);
          } 
        } else {
          // p is legal move in the probTable
          if (board->getNeighborEmptyCount(p) == 0 && board->checkLegalHand(p, player, Board::flipColor(player)) != Board::PUT_LEGAL) {
            // actually illegal
            toBeIllegal.insert(p,p);
          } else {
            allLegalMoves.insert(p,p);
          }
        }
      }
    }
  }

  void SoftmaxPolicyPlayout::enumerateMovesOfPatternChanged(Common::PointSet &addTo, const Go::Board *board, Common::Color player, const Go::Board::MoveChangeEntry *lastMove, const Go::Board::MoveChangeEntry *secondLastMove, bool doClear) {
    if (doClear) {
      addTo.clear();
    }
    if (lastMove) {
      if (lastMove->m_putPos != PASS) {
        //StandardFeatureExtractor::clearStaticFeatures(featureTable[lastMove->m_putPos]);
      }
      StandardFeatureExtractor::enumerateMovesOfPatternChanged(board, lastMove, player, addTo, false);
    }
    if (secondLastMove != NULL) {
      if (secondLastMove->m_putPos != PASS) {
        //StandardFeatureExtractor::clearStaticFeatures(featureTable[secondLastMove->m_putPos]);
      }
      StandardFeatureExtractor::enumerateMovesOfPatternChanged(board, secondLastMove, player, addTo, false);
    }
  }


  Point SoftmaxPolicyPlayout::selectOneMoveAccordingToProbabilityTable(Go::Board *board, double *table) {
    double rnd_1 = m_rnd.from_0_to_1();
    double rnd_2 = m_rnd.from_0_to_1();

    double sum_of_xdirs[40];
    double sum_of_total = 0;
    for (int y=0; y<board->getSize(); y++) {
      sum_of_xdirs[y] = 0;
      for (int x=0; x<board->getSize(); x++) {
        sum_of_xdirs[y] += table[board->xyToPoint(x,y)];
      }
      sum_of_total += sum_of_xdirs[y];
    }
    if (sum_of_total < 0.0001) { // very small => no candidates
      return PASS;
    }

    // select y
    double select_y_value = rnd_1 * sum_of_total;
    double accumulated_values = 0;
    int selected_y = -1;
    for (int y=0; y<board->getSize(); y++) {
      if (accumulated_values + sum_of_xdirs[y] > select_y_value) {
        selected_y = y;
        break;
      }
      accumulated_values += sum_of_xdirs[y];
    }
    if (rnd_1 >= 0.999999 && selected_y == -1) {
      selected_y = board->getSize() - 1;
      while (selected_y >= 0 && sum_of_xdirs[selected_y] < 0.000001) selected_y--;
    }
    assert(selected_y != -1);

    //cerr << "rnd1 = " << rnd_1 << " y = " << selected_y << endl;

    // select x
    double select_x_value = rnd_2 * sum_of_xdirs[selected_y];
    accumulated_values = 0;
    int selected_x = -1;
    for (int x=0; x<board->getSize(); x++) {
      double value = table[board->xyToPoint(x,selected_y)];
      if (value + accumulated_values > select_x_value) {
        selected_x = x;
        break;
      }
      //cerr << "value + accumulated_values = " << value + accumulated_values << endl;
      accumulated_values += value;
    }
    if (rnd_2 >= 0.999999 && selected_x == -1) {
      selected_x = board->getSize() - 1;
      while (selected_x >= 0 && table[board->xyToPoint(selected_x, selected_y)] < 0.000001) selected_x--;
    }

#ifdef DEBUG
    if (selected_x == -1)
    {
      cerr << "turn = " << (table == m_probTableBlack ? "black" : "white") << endl;
      cerr << "selected y = " << selected_y << " rnd1 = " << rnd_1 << "," << select_y_value << " rnd2 = " << rnd_2 << "," << select_x_value << endl;
      board->printToErr();
      printProbabilityTableToErr(board);
    }
#endif
    assert(selected_x != -1);

    //cerr << "rnd2 = " << rnd_2 << " x = " << selected_x << endl;

    return board->xyToPoint(selected_x, selected_y);
  }
  
  double SoftmaxPolicyPlayout::operator()(Common::Color turn_color, Go::Board *board, int depth, bool doUndo) {

    //initProbabilities(board);

    const static int playing_count_offset = 200;

    m_sequence.resize(0);
    
    int currentTurn = turn_color;
    int board_size = board->getSize();
    int max_playing = board_size*board_size + playing_count_offset;
    Common::Point prev_hand(POINT_NULL);

    // enumerate hand candidates
    int playoutHandCount = 0;

    int table_index = turn_color == BLACK ? 0 : 1;
    double *tables[] = {m_probTableBlack, m_probTableWhite};
    bool doublepass = false;

    // first 2 moves
    for (int i=0; i<2; i++)
    {
      initProbabilities(board, currentTurn, tables[table_index], currentTurn == BLACK ? m_featureTableBlack : m_featureTableWhite);
      Point selected_hand = selectOneMoveAccordingToProbabilityTable(board, tables[table_index]);
      board->put(selected_hand, currentTurn, true);

      m_sequence.push_back(selected_hand);

      if (prev_hand == PASS && selected_hand == PASS) {
        doublepass = true;
      }

      prev_hand = selected_hand;
      table_index = 1-table_index;
      currentTurn = Board::flipColor(currentTurn);
      playoutHandCount++;
    }

    //cerr << "-------------" <<  endl;
    for (; playoutHandCount<max_playing && !doublepass; playoutHandCount++) {

      // select hand according to the table
      updateProbabilitiesBeforeAction(board, currentTurn);
      // if (playoutHandCount >= 63 && playoutHandCount <= 96) {
      //   board->printToErr();
      //   printProbabilityTableToErr(board);
      // }
      
      Point selected_hand = selectOneMoveAccordingToProbabilityTable(board, tables[table_index]);
      
      Board::PutType err;
#ifdef DEBUG
      if ((err = board->checkLegalHand(selected_hand, currentTurn, Board::flipColor(currentTurn))) != Board::PUT_LEGAL) {
        board->printToErr();
        cerr << "selected move = " << board->pointToXY(selected_hand).first << "," << board->pointToXY(selected_hand).second << " turn = " << (currentTurn == BLACK ? "black" : "white") << " error = " << err << endl;
        cerr << "sequence new to old" << endl;
        for (int i=board->getHistoryCount(); i>=0; i--) {
          const Board::MoveChangeEntry *entry = board->getHistory(i);
          if (entry) {
            int killedBlocks = 0;
            for (killedBlocks=0; entry->m_killedBlocks[killedBlocks] && killedBlocks < 4; killedBlocks++) {}
            if (killedBlocks > 0) {
              int x = board->pointToXY(entry->m_putPos).first;
              int y = board->pointToXY(entry->m_putPos).second;
              cerr << "last " << i << " move = " << x << "," << y << " killed blocks = " << killedBlocks << " put color = " << entry->m_putColor << endl;
            }
          }
        }
        printProbabilityTableToErr(board);
        tables[table_index][selected_hand] = 0;
      }
#endif
      err = board->put(selected_hand, currentTurn, true);

      assert (err == Board::PUT_LEGAL);
      m_sequence.push_back(selected_hand);

      // double pass
      if (selected_hand == PASS && prev_hand == PASS) {
        // end game
        doublepass = true;
      } else {      
        prev_hand = selected_hand;
        table_index = 1-table_index;
        currentTurn = Board::flipColor(currentTurn);
      }
    }

    double score = board->countScore(turn_color);
    if (doUndo) {
      // undo changes
      for (int i=0; i<playoutHandCount; i++) {
        board->undo();
      }
    }
    //std::cerr << playoutHandCount << endl;
    return score;    
  }

  void SoftmaxPolicyPlayout::printProbabilityTableToErr(const Board *board) const {
    cerr << "BLACK:" << endl;
    for (int y=0; y<9; y++) {
      double sum = 0;
      for (int x=0; x<9; x++) {
        cerr << m_probTableBlack[board->xyToPoint(x,y)] << ",";
        sum += m_probTableBlack[board->xyToPoint(x,y)];
      }
      cerr << "|" << sum << endl;
    }
    cerr << "WHITE:" << endl;
    for (int y=0; y<9; y++) {
      double sum = 0;
      for (int x=0; x<9; x++) {
        cerr << m_probTableWhite[board->xyToPoint(x,y)] << ",";
        sum += m_probTableWhite[board->xyToPoint(x,y)];
      }
      cerr << "|" << sum << endl;
    }
  }
}
