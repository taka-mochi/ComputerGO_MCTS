
#pragma once

#include "precomp.h"
#include "AI/PlayerBase.h"
#include "AI/UCTNode.h"
#include "AI/UCTNodeAllocator.h"
#include "AI/Parameters.h"
#include "AI/HistoryHeuristics.h"
#include "AI/HistoryHeuristicsPlayout.h"
#include "utility/utility.h"

#include "ML/StandardFeatureExtractor.h"

#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace std;
using namespace Common;
using namespace Go;

#ifdef DEBUG
#define UCTPLAYER_NOINLINE __attribute__((noinline))
#else
#define UCTPLAYER_NOINLINE
#endif

namespace AI {
  
  template <class PLAYOUT_POLICY_CLASS, class UCT_BOUND_CLASS> class UCTPlayer : public PlayerBase {
    mutable int m_playoutCount;
    int m_playoutLimitForEachSelection;
    Go::Board *m_board;
    Parameters m_parameters;
    PLAYOUT_POLICY_CLASS playoutPolicy;
    UCT_BOUND_CLASS uctBoundCalculator;
    MTRandom &m_rnd;

    int m_debugLevel;

    bool m_bShouldResign;

    // some parameters for UCT search
    const int m_treeExpandPlayoutThreshold;
    bool m_doUpdateRave;
    bool m_doReuseTree;
    bool m_doDynamicChangePolicy;

    // for proposed method variables (dynamic policy update)
    ML::FeatureWeights m_initSimPolicyFeatureWeights;
    ML::FeatureWeights m_alphaLambdaInitSimFeatureWeights;
    ML::FeatureWeights m_alphaLambdaCurrentFeatureWeights;
    ML::FeatureWeights m_expCurrentFeatureWeights;
    ML::FeatureWeights m_accumulatedDerivinationPart;
    int m_totalDynamicUpdatedMovesCount;

    // for History Heuristics
    HistoryHeuristics m_history;
    const double m_initialValueOfHistory;
    bool m_doUpdateHistoryHeuristics;

    // for tree reuse
    int m_historyIndexOfLastSelectedMove;
    UCTNode::UCT_NODE_ID m_lastTreeRoot;
    
    //std::vector<double> m_tmpChildrenUCBs; // for calculation temporal various

    std::vector<Point> m_sequenceInTree;

    // NodeAllocator
    UCTNodeAllocator m_alloc;

    //
    // **** test mode flag
    //
    bool HISTORY_MODE_PRELIMINARY;

  public:
  UCTPlayer(Go::Board *init_board, PLAYOUT_POLICY_CLASS policy, const AI::Parameters &params, MTRandom &rnd)
    : m_playoutCount(0)
      , m_playoutLimitForEachSelection(params.getPlayoutLimit())
      , m_board(init_board)
      , m_parameters(params)
      , playoutPolicy(policy)
      , uctBoundCalculator(params)
      , m_rnd(rnd)
      , m_debugLevel(0)
      , m_bShouldResign(false)
      , m_treeExpandPlayoutThreshold(params.getThreshold_Expand())
      , m_doUpdateRave(true)
      , m_doReuseTree(true)
      , m_doDynamicChangePolicy(true)
      , m_initSimPolicyFeatureWeights()
      , m_alphaLambdaInitSimFeatureWeights()
      , m_expCurrentFeatureWeights()
      , m_accumulatedDerivinationPart()
      , m_totalDynamicUpdatedMovesCount(0)
      , m_history((init_board->getSize()+2)*(init_board->getSize()+2), HistoryHeuristics::HISTOGRAM, 2, Common::HISTORY_TABLE_INIT_VALUE)
      , m_initialValueOfHistory(Common::HISTORY_TABLE_INIT_VALUE)
      , m_doUpdateHistoryHeuristics(true)
      , m_historyIndexOfLastSelectedMove(-1)
      , m_lastTreeRoot(UCTNode::INVALID_NODE_ID)
      , m_sequenceInTree()
      , m_alloc((init_board->getSize()+2)*(init_board->getSize()+2))
    {
      m_sequenceInTree.reserve(MAX_BOARD_SIZE+200);
      if (playoutPolicy.getFeatureWeights() != NULL) {
        m_initSimPolicyFeatureWeights = *policy.getFeatureWeights();
        m_alphaLambdaInitSimFeatureWeights = m_initSimPolicyFeatureWeights * m_parameters.getDynamicUpdatePolicyAlpha() * m_parameters.getDynamicUpdatePolicyLambda();
        m_alphaLambdaCurrentFeatureWeights = m_alphaLambdaInitSimFeatureWeights;
        m_expCurrentFeatureWeights.init(m_initSimPolicyFeatureWeights.size());
        m_accumulatedDerivinationPart.init(m_initSimPolicyFeatureWeights.size());

        cerr << "Alpha = " << m_parameters.getDynamicUpdatePolicyAlpha() << endl;
        cerr << "Lambda = " << m_parameters.getDynamicUpdatePolicyLambda() << endl;
        cerr << "Alpha * Lambda = " << m_parameters.getDynamicUpdatePolicyLambda() * m_parameters.getDynamicUpdatePolicyAlpha() << endl;

      } else {
        m_doDynamicChangePolicy = false;
      }

      // very adhoc implementation for history heuristics playout
      if (playoutPolicy.getName() == HistoryHeuristicsPlayout::getName()) {
        HistoryHeuristicsPlayout *p = dynamic_cast<HistoryHeuristicsPlayout*>(&playoutPolicy);
        p->setHistoryHeuristics(&m_history);
        m_history.setHistogramAddValue(params.getHistoryHistogramAddValue());
        m_history.clearHistory(BLACK, m_initialValueOfHistory); m_history.clearHistory(WHITE, m_initialValueOfHistory);
        //m_history.recordHistory(m_board->xyToPoint(0,8), BLACK, 0);
        HISTORY_MODE_PRELIMINARY = true;
      } else {
        HISTORY_MODE_PRELIMINARY = false;
      }
    }

    ~UCTPlayer() {
      m_alloc.releaseAll();
      m_lastTreeRoot = UCTNode::INVALID_NODE_ID;
    }

    std::string getAIName() const {
      return "UCTPlayer-with-" + uctBoundCalculator.getName() + (m_doReuseTree ? "reuTree" : "") + "_sim-" + playoutPolicy.getName() + (m_doDynamicChangePolicy ? "_dynamic" : "") + (m_parameters.getUseOpeningBook() ? "_withbook" : "");
    }

    void setDebugLevel(int level) {
      if (level <= 0) m_debugLevel = 0;
      else m_debugLevel = level;
    }

    void setReuseTree(bool reuse) {
      m_doReuseTree = reuse;
    }

    void setDynamicPolicyUpdate(bool doUpdate) {
      m_doDynamicChangePolicy = doUpdate && (playoutPolicy.getFeatureExtractor() != NULL);
    }

    bool getDynamicPolicyUpdate() const {
      return m_doDynamicChangePolicy;
    }

    inline bool shouldResign() const {
      return m_bShouldResign;
    }

    Common::Point selectBestMove(Color turn) {
      cerr << "Top of Select Best Move" << endl;
      cerr.flush();
      // get root node
      UCTNode::UCT_NODE_ID rootID = UCTNode::INVALID_NODE_ID;

      clock_t beginOfReleaseSync = clock();
      m_alloc.waitAsyncNodeReleasingFinish();
      if (m_doReuseTree) {
        cerr << "Last Root ID = " << m_lastTreeRoot << endl;
        rootID = findReuseTreeRoot(m_lastTreeRoot);
        if (rootID != UCTNode::INVALID_NODE_ID) {
          cerr << "!!Reusable Tree is Found!!" << endl;
        }
      }

      if (rootID == UCTNode::INVALID_NODE_ID) {
        UCTNode *last = m_alloc.get(m_lastTreeRoot);
        if (last && last->getId() != UCTNode::INVALID_NODE_ID) {
          m_alloc.release(m_lastTreeRoot, true);
        } 
        rootID = m_alloc.create(POINT_NULL);
      }
      m_alloc.startAsyncNodeReleasing();
#ifdef DEBUG
      cerr << "-------------- Time for Release Sync and other = " << ((double)(clock()-beginOfReleaseSync)/CLOCKS_PER_SEC*1000) << endl;

      cerr << "RootID = " << rootID << endl;
#endif
      UCTNode *rootNode = m_alloc.get(rootID);

      if (!rootNode->areChildrenExpanded()) {
        createChildren(rootNode, turn);
      }

      int startPlayoutCount = m_playoutCount;

      clock_t start = 0, end = 0;
      if (m_debugLevel >= 1) {
        start = clock();
      }

      // do uct
      Point preliminary_best_p = POINT_NULL;
      if (HISTORY_MODE_PRELIMINARY) {
        static HistoryHeuristics tmpHistory(m_history.getTableSize());
        tmpHistory.clearHistory(BLACK); tmpHistory.clearHistory(WHITE);
        HistoryHeuristicsPlayout *p = dynamic_cast<HistoryHeuristicsPlayout*>(&playoutPolicy);
        // set the NULL history heuristic table
        p->setHistoryHeuristics(&tmpHistory);
        p->setAvailabilityOfHistoryValueConvergence(false);
        
        m_playoutLimitForEachSelection *= 10;
        
        UCTNode::UCT_NODE_ID rootForPreliminary = m_alloc.create(POINT_NULL);
        createChildren(m_alloc.get(rootForPreliminary), turn);
        if (!searchUCT_entryPoint(turn, rootForPreliminary)) {
          preliminary_best_p = PASS;
        } else {
          UCTNode *bestNode = findMaxGameCountChildNode(m_alloc.get(rootForPreliminary));
          preliminary_best_p = bestNode->getPos();
        }
        cerr << "*** Best move got in advance = " << m_board->pointToXY(preliminary_best_p).first << "," << m_board->pointToXY(preliminary_best_p).second << endl;
        
        m_alloc.release(rootForPreliminary, true, false);
        m_history.clearHistory(BLACK, m_initialValueOfHistory); m_history.clearHistory(WHITE, m_initialValueOfHistory);
        if (preliminary_best_p != PASS) {
          m_history.recordHistory(preliminary_best_p, turn, 1);
          p->setRequiredPatternToChangeProbabilityFromBoard(preliminary_best_p, turn, m_board);
        }

        cerr << "Preliminary Code" << endl;
        p->setHistoryHeuristics(&m_history);        
        p->setAvailabilityOfHistoryValueConvergence(true);
        p->printProbabilityTableToErr(m_board);
        p->clearStatistics();
        m_playoutLimitForEachSelection /= 10;
      }
      
      if (searchUCT_entryPoint(turn, rootID) == false) {
        return POINT_NULL;
      }
      cerr << "Checked Count/Total Count = " << debug_checkedCount << "/" << debug_totalCount << endl;

      if (HISTORY_MODE_PRELIMINARY) {
        if (preliminary_best_p != PASS) {
          dynamic_cast<HistoryHeuristicsPlayout*>(&playoutPolicy)->disableReuiredPatternToChangeProbability(preliminary_best_p, turn);
          HistoryHeuristicsPlayout *p = dynamic_cast<HistoryHeuristicsPlayout*>(&playoutPolicy);
          p->printStatisticsToErr();
          p->clearStatistics();
        }
      }

      if (m_debugLevel >= 1) {
        end = clock();
        int diffPlayoutCount = m_playoutCount - startPlayoutCount;
        double time_diff_msec = ((double)(end-start)/CLOCKS_PER_SEC*1000);
        cerr << "Executed playout count = " << diffPlayoutCount << endl;
        cerr << "Time per playout (msec) = " << time_diff_msec/diffPlayoutCount << endl;
        cerr << "Playout speed (playout/sec) = " << 1000*diffPlayoutCount/time_diff_msec << endl;
      }

      // select a node which is most visited
      UCTNode *max_node = findMaxGameCountChildNode(rootNode);
      //UCTNode *max_node = findMaxWinrateChildNode(rootNode);

      /* int max_visited = 0; */
      /* const std::vector<UCTNode::UCT_NODE_ID> &children = rootNode->getChildren(); */
      /* for (std::vector<UCTNode::UCT_NODE_ID>::const_iterator it = children.begin(); */
      /*      it != children.end(); it++) { */
      /*   if (*it == UCTNode::INVALID_NODE_ID) continue; */
      /*   UCTNode *node = m_alloc.get(*it); */
      /*   assert(node); */
      /*   if (node->getPos() == POINT_NULL) continue; */
      /*   if (max_visited < node->getGameCount()) { */
      /*     max_visited = node->getGameCount(); */
      /*     max_node = node; */
      /*   } */
      /* } */

      double win_rate = max_node->getWinCount() / max_node->getGameCount();
      if (turn == WHITE) {
        win_rate += 1.0;
      }
      cerr << "Max Visited = " << m_board->pointToXY(max_node->getPos()).first << "," << m_board->pointToXY(max_node->getPos()).second << " count = " << max_node->getGameCount() << " winrate = " << win_rate << " UCB = " << calculateUCBValue(max_node, rootNode->getGameCount()) << " RAVE = " << calculateRaveValue(max_node, rootNode->getChildrenTotalRaveCount()) << endl;
      double b, w;
      m_board->countScoreDetail(b,w);
      cerr << "Scores: b = " << b << " w = " << w << endl;


      // update last search information
      updateLastSearchInformation(rootID);

      m_bShouldResign = false;
      if (m_parameters.isResignAllowed()) {
        // resign check
        if (win_rate < m_parameters.getResignThresholdOfWinRate()) {
          m_bShouldResign = true;
          cerr << "Should Resign" << endl;
        }
      }
      
      return max_node->getPos();
    }

  private:
    UCTNode::UCT_NODE_ID findReuseTreeRoot(UCTNode::UCT_NODE_ID oldTreeRoot) {
      // Reuse Search Tree
      if (m_historyIndexOfLastSelectedMove < 0 ||
          oldTreeRoot == UCTNode::INVALID_NODE_ID ||
          m_board->getHistoryCount() == 0) {
        return UCTNode::INVALID_NODE_ID;
      }
      UCTNode *oldTree = m_alloc.get(oldTreeRoot);
      if (!oldTree || oldTree->getId() == UCTNode::INVALID_NODE_ID) {
        return UCTNode::INVALID_NODE_ID;
      }
      
      // get history index of the previous move
      int last_move_index = m_board->getHistoryCount()-1;
      // make a sequence btw tree
      std::vector<Common::Point> seq;
      seq.reserve(last_move_index - m_historyIndexOfLastSelectedMove + 1);
      cerr << "seq:";
      for (int i=m_historyIndexOfLastSelectedMove; i<=last_move_index; i++) {
        Common::Point p = m_board->getMoveHistory(i);
        assert(p != POINT_NULL);
        cerr << p << ",";
        seq.push_back(p);
      }
      cerr << endl;
      // find reuse tree
      return oldTree->findTreeRootToReuse(seq);
    }

    int debug_checkedCount;
    int debug_totalCount;

    bool searchUCT_entryPoint(Color turn, UCTNode::UCT_NODE_ID rootId) {

      m_board->takeSnapshot();
      debug_totalCount = 0;
      debug_checkedCount = 0;
      int startPlayoutCount = m_playoutCount;
      UCTNode *rootNode = m_alloc.get(rootId);

      /* if (m_doUpdateHistoryHeuristics) { */
      /*   m_history.clearHistory(BLACK); m_history.clearHistory(WHITE); */
      /* } */

      while (m_playoutCount < startPlayoutCount + m_playoutLimitForEachSelection) {

        //int updateMoveCountBegin = m_totalDynamicUpdatedMovesCount;
        if (m_doDynamicChangePolicy) {
          const ML::FeatureWeights *weights = playoutPolicy.getFeatureWeights();
          m_alphaLambdaCurrentFeatureWeights = *weights;
          m_alphaLambdaCurrentFeatureWeights *= m_parameters.getDynamicUpdatePolicyLambda() * m_parameters.getDynamicUpdatePolicyAlpha();
          //m_expCurrentFeatureWeights = *playoutPolicy.getExpFeatureWeights();
          for (size_t i=0; i<weights->size(); i++) {
            m_expCurrentFeatureWeights[i] = fmath::expd(weights->at(i));
          }
          m_accumulatedDerivinationPart.init(weights->size());
        }


        m_sequenceInTree.resize(0);
        int total_repetition = 0;
        int win = -searchUCT(turn, rootId, 1, false, total_repetition);
        if (abs(win)+1 >= NO_GAME_WIN_RETURN) {
          return false;
        }
        backupProcess(rootNode, NULL, NULL, turn, 1, win);
        m_board->restoreStateFromSnapshot();

        if (m_doDynamicChangePolicy) {
          //int updatedMovesCount = m_totalDynamicUpdatedMovesCount - updateMoveCountBegin;
          //cerr << "Updated Moves: " << updatedMovesCount << endl;
          //cerr << "Cummulative: " << m_accumulatedDerivinationPart.toString() << endl;

          //m_alphaLambdaCurrentFeatureWeights -= m_alphaLambdaInitSimFeatureWeights;
          //m_alphaLambdaCurrentFeatureWeights *= static_cast<double>(updatedMovesCount);
          m_accumulatedDerivinationPart.minusDiffOfTwoInstances(m_alphaLambdaCurrentFeatureWeights, m_alphaLambdaInitSimFeatureWeights);
          //m_accumulatedDerivinationPart -= m_alphaLambdaCurrentFeatureWeights;
          playoutPolicy.addFeatureWeights(m_accumulatedDerivinationPart);
        }

      }
      return true;
          /* cerr << "weights[0]: " << (*playoutPolicy.getFeatureWeights())[0] << endl; */
          /* cerr << "weights[1]: " << (*playoutPolicy.getFeatureWeights())[1] << endl; */
          /* cerr << "weights[2]: " << (*playoutPolicy.getFeatureWeights())[2] << endl; */
          /* cerr << "weights[3]: " << (*playoutPolicy.getFeatureWeights())[3] << endl; */
      //if (playoutPolicy.getFeatureWeights() != NULL) {
      //cerr << "weight: " << playoutPolicy.getFeatureWeights() << endl;
      //}

      // dynamic update policy
    }
    static const int NO_GAME_WIN_RETURN = 99999;

    int searchUCT(Color turn, UCTNode::UCT_NODE_ID targetNode, int depth, bool doUndo, int &total_repetition_count)
    {
      assert(targetNode != UCTNode::INVALID_NODE_ID);
      
      UCTNode *node = m_alloc.get(targetNode);
      assert(node);

      UCTNode *max_ucb_node = NULL, *second_max_ucb_node = NULL;

      // if the children of node are expanded only by RAVE, expand all other unexpanded children
      if (!node->areChildrenExpanded() && node->getGameCount() > m_treeExpandPlayoutThreshold) {
        createChildren(node, turn);
      }

      // select a node with the highest UCB value
      int count = 0;
      bool isSpecialPattern = false;
      
      // ----- variables for proposed method -----
      SparseVector featureTable[MAX_BOARD_SIZE], passFeature;
      PointSet allset, added;
      // -----------------------------------------

      if (doUpdateDynamicallyTheNode(node)) {
        // for proposed method
        ML::StandardFeatureExtractor *extractor = this->playoutPolicy.getFeatureExtractor();
        extractor->extractFromStateForAllMoves(m_board, turn, featureTable, passFeature, allset, true);
      }

      int repetition_count = 0;
      do {
        max_ucb_node = findMaxUCBChildNode(node, &second_max_ucb_node);
        //max_ucb_node = findMaxChildNodeByPrecomputedUCB(node, m_tmpChildrenUCBs);
        if (max_ucb_node == NULL) {
          // this node can select only PASS
          UCTNode::UCT_NODE_ID newid = m_alloc.create(PASS);
          node->putChild(PASS, newid);
          max_ucb_node = m_alloc.get(newid);
          //cerr << "PASS parent " << node->getPos() << endl;
        }

        assert (max_ucb_node->getId() != UCTNode::INVALID_NODE_ID);

        //cout << max_ucb_node->getPos() << endl;
        
        count++;
#ifdef DEBUG
        bool tmp = false;
        if ( m_board->put(max_ucb_node->getPos(), turn) != Board::PUT_LEGAL || 
             (max_ucb_node->getGameCount() >= 1 && max_ucb_node->getPos() != PASS && (tmp = m_board->isRepeatedPosition())) ) {
          if (!(tmp && max_ucb_node->getPos()== PASS)) {

            cerr << "This NODE is ALREADY SIMULATED but '''INVALID NODE'''. it must be error..." << endl;
            cerr << "Game Count = " << max_ucb_node->getGameCount() << endl;
            Point p = max_ucb_node->getPos();
            cerr << "put = " << m_board->pointToXY(p).first << "," << m_board->pointToXY(p).second << " repetition = " << tmp << " turn = " << turn << endl;
            cerr << "parent put = " << node->getPos() << endl;
            cerr << "playout = " << m_playoutCount << endl;
            //cerr << "ucb = " << uctBoundCalculator(max_ucb_node, node) << endl;
            m_board->printToErr();
            assert(false);
          }
        }
        m_board->undo();
        debug_totalCount++;
#endif
        bool isLegalHand;
        bool alreadyPut = false;
        checkLegalityOfTheNode(turn, max_ucb_node, isLegalHand, alreadyPut, isSpecialPattern);

        if (!isLegalHand) {
          // invalid move or board repetition
          // force to select other move
          if (alreadyPut && max_ucb_node->getPos() == PASS) {
            // special pattern
            // this pattern cannot lead to end of game
            // forcely exit
            isSpecialPattern = true;
          } else {
            if (alreadyPut) {
              cerr << m_board->pointToXY(max_ucb_node->getPos()).first << "," << m_board->pointToXY(max_ucb_node->getPos()).second << ". Repetition Count = " << total_repetition_count << ". Depth = " << depth << endl;
              m_board->printToErr();
              m_board->undo();
              cerr << "Repetition!!" << endl;
              total_repetition_count++;
              repetition_count++;
              if (total_repetition_count > 30) {
                cerr << "!!!!!!!!!!!!!!!!!!!!!!!! No Game !!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                 return NO_GAME_WIN_RETURN;
              }
            }
            max_ucb_node->setToInvalidMove();          
            max_ucb_node = NULL;
            if (repetition_count >= 5) {
              UCTNode::UCT_NODE_ID newid = m_alloc.create(PASS);
              node->putChild(PASS, newid);
              max_ucb_node = m_alloc.get(newid);
            }
          }
        }

      } while (max_ucb_node == NULL);

      m_sequenceInTree.push_back(max_ucb_node->getPos()); // update move sequence

      double win = 0;

      // if this node is PASS and selected child is also PASS, double pass (end game)
      if (isSpecialPattern || (node->getPos() == PASS && max_ucb_node->getPos() == PASS)) {
        // do not deepen tree
        m_playoutCount++;
        win = -m_board->countScore(Board::flipColor(turn));
      } else {
        // do a playout or deepen tree
        if (max_ucb_node->getGameCount() <= m_treeExpandPlayoutThreshold) {
          m_playoutCount++;
          win = -playoutPolicy(Board::flipColor(turn), m_board, depth+1, doUndo);
        } else {
          win = -searchUCT(Board::flipColor(turn), max_ucb_node->getId(), depth+1, doUndo, total_repetition_count);
        }
      }
      if (fabs(win)+1>=NO_GAME_WIN_RETURN) {
        return NO_GAME_WIN_RETURN;
      }

      if (doUpdateDynamicallyTheNode(node)) {
        // proposed method
        updateSimulationPolicyDynamically(max_ucb_node->getPos(), featureTable, passFeature, allset);
      }

      if (doUndo) m_board->undo();      
      // win is wincount for turn of node.
      // -win is wincount for turn of max_ucb_node
      // => node wants to select highest win
      //    max_ucb_node wants to select lowest win (mini-max theory)
      //    therefore, win is set to child of node, max_ucb_node
      //    (sign of node's winrate is adjust to parent's turn)
      backupProcess(max_ucb_node, node, second_max_ucb_node, turn, depth+1, win);

      return win;
    }

    class GameCountAccessor {
    public:
      int operator()(UCTNode *node) const {
        return node->getGameCount();
      }
    };
    class WinrateAccessor {
    public:
      double operator()(UCTNode *node) const {
        return static_cast<double>(node->getWinCount())/node->getGameCount();
      }
    };
    template <typename TYPE, typename GET_CLASS> UCTNode *findMaxChildNode(UCTNode *node, const GET_CLASS &accessor, UCTNode **secondMaxNodeBuf = NULL) {
      UCTNode *max_node = NULL;
      TYPE max_count = -99999;
      TYPE second_max_count = max_count - 1;

      const UCTNode::UctNodeIdList &children = node->getChildren();
      for (UCTNode::UctNodeIdList::const_iterator it = children.begin();
           it != children.end(); it++) {
        UCTNode::UCT_NODE_ID id = *it;
        //if (id == UCTNode::INVALID_NODE_ID) continue;
        UCTNode *child = m_alloc.get(id);
        assert(child);
        //if (child->getId() == UCTNode::INVALID_NODE_ID) continue;
        assert(id != UCTNode::INVALID_NODE_ID);
        assert(child->getId() != UCTNode::INVALID_NODE_ID);
        
        if (child->getPos() == POINT_NULL) continue;
        TYPE count = accessor(child);
        
        if (max_count < count) {
          if (secondMaxNodeBuf) {
            *secondMaxNodeBuf = max_node;
            second_max_count = max_count;
          }
          max_count = count;
          max_node = child;
        } else if (secondMaxNodeBuf) {
          if (second_max_count < count) {
            *secondMaxNodeBuf = child;
            second_max_count = count;
          }
        }
      }
      return max_node;      
    }

    UCTNode *findMaxGameCountChildNode(UCTNode *node, UCTNode **secondMaxNodeBuf = NULL) UCTPLAYER_NOINLINE {
      return findMaxChildNode<int, GameCountAccessor>(node, GameCountAccessor(), secondMaxNodeBuf);
    }
    UCTNode *findMaxWinrateChildNode(UCTNode *node, UCTNode **secondMaxNodeBuf = NULL) UCTPLAYER_NOINLINE {
      return findMaxChildNode<double, WinrateAccessor>(node, WinrateAccessor(), secondMaxNodeBuf);
    }

    // select a node with the highest UCB value
    UCTNode *findMaxUCBChildNode(UCTNode *node, UCTNode **secondMaxNodeBuf = NULL) UCTPLAYER_NOINLINE {
      UCTNode *max_ucb_node = NULL;
      double max_ucb = -99999;
      double second_max_ucb = max_ucb - 1;

      double parentGameCountLog = node->getGameCount() > 0 ? fmath::log(node->getGameCount()) : 0.0;
      double parentRaveCountLog = node->getRaveCount() > 0 ? fmath::log(node->getRaveCount()) : 0.0;

      const UCTNode::UctNodeIdList &children = node->getChildren();
      for (UCTNode::UctNodeIdList::const_iterator it = children.begin();
           it != children.end(); it++) {
        UCTNode::UCT_NODE_ID id = *it;
        //if (id == UCTNode::INVALID_NODE_ID) continue;
        UCTNode *child = m_alloc.get(id);
        assert(child);
        //if (child->getId() == UCTNode::INVALID_NODE_ID) continue;
        assert(id != UCTNode::INVALID_NODE_ID);
        assert(child->getId() != UCTNode::INVALID_NODE_ID);
        
        if (child->getPos() == POINT_NULL) continue;
        double ucb = 0.0;
        if (child->getGameCount() == 0) {
          ucb = 10000 + m_rnd()%50000;
        } else {

          //ucb = uctBoundCalculator(child, node);//parentGameCountLog, parentRaveCountLog);
          ucb = uctBoundCalculator(child, parentGameCountLog, parentRaveCountLog, node);
        }
        
        if (max_ucb < ucb) {
          if (secondMaxNodeBuf) {
            *secondMaxNodeBuf = max_ucb_node;
            second_max_ucb = max_ucb;
          }
          max_ucb = ucb;
          max_ucb_node = child;
        } else if (secondMaxNodeBuf) {
          if (second_max_ucb < ucb) {
            *secondMaxNodeBuf = child;
            second_max_ucb = ucb;
          }
        }
      }
      return max_ucb_node;
    }

    inline double calculateUCBValue(UCTNode *node, int total_game_count) {
      double gameCountInv = 1.0/node->getGameCount();
      return node->getWinCount()*gameCountInv + m_parameters.getUCT_C() * sqrt(log(total_game_count)*gameCountInv);
    }
    inline double calculateRaveValue(UCTNode *node, int total_rave_count) {
      double raveCountInv = 1.0/node->getRaveCount();
      return node->getRaveWinCount()*raveCountInv + m_parameters.getUCT_C() * sqrt(log(total_rave_count)*raveCountInv);
    }
    inline double calculateUCB_RAVE(UCTNode *node, int total_game_count, int total_rave_count) {
      const double K = m_parameters.getRave_K();
      const double C = m_parameters.getUCT_C();
      const double M = m_parameters.getRave_M();
      double ucb = calculateUCBValue(node, total_game_count);
      double rave = calculateRaveValue(node, total_rave_count);
      double beta = sqrt(K/(K+M*total_game_count));
      return (1-beta)*ucb + beta*rave;
    }

    inline void checkLegalityOfTheNode(Color turn, UCTNode *max_ucb_node, bool &isLegalHand, bool &alreadyPut, bool &isSpecialPattern) {
      alreadyPut = false;
      isSpecialPattern = false;
      if (max_ucb_node->getGameCount() == 0) {
        isLegalHand = false;
        //if (m_board->checkLegalHand(max_ucb_node->getPos(), turn, Board::flipColor(turn)) == Board::PUT_LEGAL) {
        m_board->put(max_ucb_node->getPos(), turn, true);
        //cerr << max_ucb_node->getPos() << endl;
        alreadyPut = true;
        if (!m_board->isRepeatedPosition()) isLegalHand = true;
        //}
      } else {
        m_board->put(max_ucb_node->getPos(), turn, true);
        isLegalHand = true;
        alreadyPut = true;
        if (max_ucb_node->getPos() == PASS) {
          if (m_board->isRepeatedPosition()) {
            isSpecialPattern = true;
            cerr << "***** Special Case ***** Turn = " << (turn == BLACK ? "black" : "white") << " *** Move = PASS " << endl;
            m_board->printToErr();
            return;
          }
        }
      }
      assert ((isLegalHand && alreadyPut) || !isLegalHand); // if the move is correct, put is already executed
    }

    // for proposed method
    bool doUpdateDynamicallyTheNode(UCTNode *node) {
      return m_doDynamicChangePolicy && node->getGameCount() > m_treeExpandPlayoutThreshold;
    }

    // for proposed method
    void updateSimulationPolicyDynamically(Point selectedMove, SparseVector *featureTable, SparseVector &passFeature, PointSet &allPossiblePoints) UCTPLAYER_NOINLINE {
      SparseVector *selectedFeature = selectedMove == PASS ? &passFeature : &featureTable[selectedMove];

      m_totalDynamicUpdatedMovesCount++;

      const ML::FeatureWeights *weights = playoutPolicy.getFeatureWeights();
      assert (weights != NULL);

      double totalOfExpValues = 0;
      double alpha = m_parameters.getDynamicUpdatePolicyAlpha();
      //double lambda = m_parameters.getDynamicUpdatePolicyLambda();

      static ML::FeatureWeights result;
      result.init(m_initSimPolicyFeatureWeights.size());
      assert (result.size() == playoutPolicy.getFeatureExtractor()->getFeatureDimension());
      assert (result.size() == m_initSimPolicyFeatureWeights.size());

      // calculate the gradients
      // 2nd term
      for (size_t i=0; i<allPossiblePoints.size(); i++) {
        Point p = allPossiblePoints[i];
        SparseVector *vec = &featureTable[p];
        double expdot = m_expCurrentFeatureWeights.multiplyAll(*vec);//exp(weights->dot(*vec));
        totalOfExpValues += expdot;

        for (size_t j=0; j<vec->size(); j++) {
          int dim = vec->at(j);
          result[dim] -= alpha * expdot;
        }
      }
      double expdot = m_expCurrentFeatureWeights.multiplyAll(passFeature);//exp(weights->dot(passFeature));
      totalOfExpValues += expdot;
      for (size_t j=0; j<passFeature.size(); j++) {
        int dim = passFeature[j];
        result[dim] -= alpha*expdot;
      }
      result *= (1.0/totalOfExpValues);
    
      //result += m_alphaLambdaInitSimFeatureWeights;
      //result -= m_alphaLambdaCurrentFeatureWeights;
      
//result -= (*weights) * alpha * lambda;
      //result += *weights;
      //result += (*weights) * (1-alpha*lambda);

      // add 1st term
      for (size_t i=0; i<selectedFeature->size(); i++) {
        int dim = selectedFeature->at(i);
        result[dim] += alpha;
      }

      /* for (size_t i=0; i<changedDimensions.size(); i++) { */
      /*   int dim = changedDimensions[i]; */
      /*   m_accumulatedDerivinationPart[dim] += result[dim]; */
      /* } */
      m_accumulatedDerivinationPart += result;
    }

    void createChildren(UCTNode *node, Color turn) {
      int board_size = m_board->getSize();
      for (int y=0; y<board_size; y++) {
        for (int x=0; x<board_size; x++) {
          Point p = m_board->xyToPoint(x,y);
          if (m_board->getStone(p) == FREE && node->getChild(p) == UCTNode::INVALID_NODE_ID &&
              (m_board->getNeighborEmptyCount(p)>=1 || m_board->checkLegalHand(p, turn, Board::flipColor(turn)) == Board::PUT_LEGAL)) {
            //m_board->checkLegalHand(p, turn, Board::flipColor(turn)) == Board::PUT_LEGAL) {
            node->putChild(p, m_alloc.create(p));
          }
        }
      }
      node->setToChildrenAreExpanded();
      //node->addChild(m_alloc.create(PASS));
    }

    void backupProcess(UCTNode *node, UCTNode *parent, UCTNode *second_max, Color turn, int depth, double win) {
      //cout << "win = " << win << " depth = " << depth << endl;
      node->updateAfterOneGame(win);
      // update rave
      if (m_doUpdateRave) {
        //////////////////////////////////////////////////////////
        // updateRave is tooooooooooo sloooooooooooowwwwwww!!!!
        //  TODO: Follow Fuego!!
        ////////////////////////////////////////////////////////
        //updateRave(node, depth, win);
      }

      if (m_doUpdateHistoryHeuristics && parent && second_max) {
        // compare the UCB/win rate between best and secondbest
        // if secondbest > best, record secondbest to HistoryHeuristics
        
        if (second_max->getGameCount() > 0) {
          // always records for best? or only when the relation is reversed???
          
          // always records: strictly, we have to check all possible moves
          
          double parentGameCountLog = fmath::log(parent->getGameCount() + 1);
          double parentRaveCountLog = parent->getRaveCount() > 0 ? fmath::log(parent->getRaveCount()) : 0.0;
          double max_ucb = uctBoundCalculator(node, parentGameCountLog, parentRaveCountLog, parent);
          double second_max_ucb = uctBoundCalculator(second_max, parentGameCountLog, parentRaveCountLog, parent);

          UCTNode *target = NULL;
          if (max_ucb >= second_max_ucb) {
            // if only consider reversed, this statement should be comment out
            //target = node;
          } else {
            target = second_max;
          }

          /* if (target && target->getPos() != PASS) { */
          /*   m_history.recordHistory(target->getPos(), turn, depth); */
          /* } */
        }
      }
    }

    void updateRave(UCTNode *parentNode, int parentDepth, double parentWin) {
      // "parentNode" selected "selectedNode" on the depth "parentDepth"
      // m_sequenceInTree.at(parentDepth-1) => selectedHand on "parentNode"
      double win = -parentWin;
      //const std::vector<UCTNode::UCT_NODE_ID> &children = parentNode->getChildren();
      const std::vector<Point> *sequences[] = {
        &m_sequenceInTree, &playoutPolicy.getLastSequence()
      };
      size_t seqIndex = parentDepth-1;
      size_t maxSeqSize = 0;
      size_t seqOffset = 0;

      static std::bitset<MAX_BOARD_SIZE> alreadyMoveFlag;
      alreadyMoveFlag = 0;

      for (int i=0; i<2; i++) {
        const std::vector<Point> *sequence = sequences[i];
        maxSeqSize += sequence->size();
        for (; seqIndex < maxSeqSize; seqIndex+=2) {
          Point hand = (*sequence)[seqIndex-seqOffset];
          if (hand == PASS) continue;

          if (alreadyMoveFlag[hand]) continue; // use only First Move in the sequence
          //if (!m_board->isFirst(hand)) continue; 
          alreadyMoveFlag.set(hand);

          UCTNode::UCT_NODE_ID id = parentNode->getChild(hand); // get child node
          UCTNode *targetNode = m_alloc.get(id);
          if (targetNode == NULL) {
            // the node does not exist
            // create the child
            
            id = m_alloc.create(hand);
            parentNode->putChild(hand, id);
            targetNode = m_alloc.get(id);
            assert(targetNode);
          } else if (targetNode->getPos() == POINT_NULL) {
            continue;
          }
          // update rave value
          // TODO: replace this calculation by class/method or something
          targetNode->updateAfterOneGameForRave(win*(1 - 0.0015*seqIndex));
          parentNode->incrementChildRaveCount();
        }
        seqOffset += sequence->size();
      }
    }

    void updateLastSearchInformation(UCTNode::UCT_NODE_ID rootID) {
      m_historyIndexOfLastSelectedMove = m_board->getHistoryCount();
      m_lastTreeRoot = rootID;
    }

  };
}
