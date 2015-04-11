
#include "precomp.h"

#include "AI/UCTNode.h"

#include <cmath>
#include <iostream>
#include <algorithm>

#include "AI/UCTNodeAllocator.h"

using namespace std;
using namespace Common;

namespace AI {
  
  // class UCTInternal {
  // public:
  //   static const int eachAllocateUCTNodeCount = 30000;
  //   static vector<UCTNode *> allNodes;
  //   //static size_t usedNodeCount;
  //   static vector<UCTNode::UCT_NODE_ID> usableIDs;
  // };

  //vector<UCTNode *> UCTInternal::allNodes;
  //size_t UCTInternal::usedNodeCount = 0;
  const UCTNode::UCT_NODE_ID UCTNode::INVALID_NODE_ID = -1;
  //vector<UCTNode::UCT_NODE_ID> UCTInternal::usableIDs;

  UCTNode::UCTNode()
    : m_alloc(NULL)
    , m_children()
    , m_insertedChildren()
  {
    init(INVALID_NODE_ID, POINT_NULL);
  }

  UCTNode::UCTNode(UCTNodeAllocator *alloc, size_t maxBoardSize)
    : m_alloc(alloc)
    , m_children()
    , m_insertedChildren()
  {
    m_children = vector<UCT_NODE_ID>();
    m_children.resize(maxBoardSize+1, INVALID_NODE_ID);

    //m_insertedChildren.reserve(maxBoardSize+1);
    init(INVALID_NODE_ID, POINT_NULL);
  }

  void UCTNode::init(UCTNodeAllocator *alloc, size_t maxBoardSize, UCT_NODE_ID boardId, Point pos) {
    m_alloc = alloc;
    m_children = vector<UCT_NODE_ID>();
    m_children.resize(maxBoardSize+1, INVALID_NODE_ID);

    init(boardId, pos);
  }

  void UCTNode::init(UCT_NODE_ID boardId, Point pos)
  {
    m_boardId = boardId;
    m_pos = pos;
    m_gameCount = 0;
    m_winCount = 0.0;
    //m_ucbValue = 0.0;
    m_raveCount = 0;
    m_raveWinCount = 0.0;
    m_raveChildrenTotalCount = 0;
    //m_childrenCount = 0;
    m_areChildrenExpanded = false;
    std::fill(m_children.begin(), m_children.end(), INVALID_NODE_ID);
    m_insertedChildren.clear();
  }

  UCTNode::~UCTNode() {
    m_children = vector<UCT_NODE_ID>();
    m_insertedChildren = vector<UCT_NODE_ID>();
  }

/*
  void UCTNode::manualInit()
  {
    if (UCTInternal::usableIDs.size()==0) {
      cerr << "UCT Node reallocation * 20 start" << endl;
      for (int i=0; i<UCTInternal::eachAllocateUCTNodeCount*20; i++) {
        UCTInternal::allNodes.push_back(new UCTNode);
        UCTInternal::usableIDs.push_back(UCTInternal::allNodes.size());
      }
      cerr << "UCT Node reallocation * 20 finished" << endl;
    }
  }

  int UCTNode::createNewNode(Point pos)
  {
    //if (UCTInternal::usedNodeCount >= UCTInternal::allNodes.size()) {
    if (UCTInternal::usableIDs.size()==0) {
      cerr << "UCT Node reallocation start" << endl;
      for (int i=0; i<UCTInternal::eachAllocateUCTNodeCount; i++) {
        UCTInternal::allNodes.push_back(new UCTNode);
        UCTInternal::usableIDs.push_back(UCTInternal::allNodes.size());
      }
      cerr << "UCT Node reallocation finished" << endl;
    }
    //UCTNode *newnode = UCTInternal::allNodes[UCTInternal::usedNodeCount++];
    UCT_NODE_ID new_id = UCTInternal::usableIDs.back();
    UCTInternal::usableIDs.pop_back();
    UCTNode *newnode = UCTInternal::allNodes[new_id-1];
    newnode->init(new_id, pos);

    return new_id;
  }

  UCTNode *UCTNode::getNode(UCT_NODE_ID id) {
    if (id < 1 || id > (signed)UCTInternal::allNodes.size()) return NULL;
    //if (id < 1 || id > static_cast<int>(UCTInternal::usedNodeCount)) return NULL;
    return UCTInternal::allNodes[id-1];
  }

  void UCTNode::releaseAllNode()
  {
    UCTInternal::usableIDs.clear();
    for (size_t i=1; i<=UCTInternal::allNodes.size(); i++) {
      UCTInternal::allNodes[i-1]->m_boardId = INVALID_NODE_ID;
      UCTInternal::usableIDs.push_back(i);
    }
    //UCTInternal::usedNodeCount = 0;
  }

  void UCTNode::releaseNode(UCT_NODE_ID id, UCT_NODE_ID parentID)
  {
    if (id<1||id>(signed)UCTInternal::allNodes.size()) return; // invalid id
    UCTNode *node = getNode(id);
    if (node->m_boardId == INVALID_NODE_ID) return;
    Common::Point p = node->getPos();
    node->m_boardId = INVALID_NODE_ID;
    UCTInternal::usableIDs.push_back(id);
    
    if (p != Board::POINT_NULL) {
      UCTNode *parent = getNode(parentID);
      if (parent) {
        parent->m_children[p+1] = INVALID_NODE_ID;
      }
    }
  }

  void UCTNode::releaseNodeAndDescendant_internal(UCTNode::UCT_NODE_ID id) {
    UCTNode *node = UCTNode::getNode(id);
    if (node->m_boardId == UCTNode::INVALID_NODE_ID) return; // already released
    node->m_boardId = UCTNode::INVALID_NODE_ID;
    const std::vector<UCTNode::UCT_NODE_ID> &children = node->getChildren();
    for (std::vector<UCTNode::UCT_NODE_ID>::const_iterator it = children.begin();
         it != children.end(); it++) {
      if (*it != UCTNode::INVALID_NODE_ID) {
        releaseNodeAndDescendant_internal(*it);
      }
    }   
    assert(std::find(UCTInternal::usableIDs.begin(), UCTInternal::usableIDs.end(), id) == UCTInternal::usableIDs.end());
    UCTInternal::usableIDs.push_back(id);
  }

  void UCTNode::releaseNodeAndDescendant(UCT_NODE_ID id, UCT_NODE_ID parentID)
  {
    UCTNode *node = getNode(id);
    if (!node) return;
    if (node->m_boardId == INVALID_NODE_ID) return; // already released

    Point p = node->getPos();
    releaseNodeAndDescendant_internal(id);
    if (p != Board::POINT_NULL) {
      UCTNode *parent = getNode(parentID);
      if (parent) {
        parent->m_children[p+1] = INVALID_NODE_ID;
      }
    }
  }

  size_t UCTNode::getNewNodeCapacity()
  {
    return UCTInternal::usableIDs.size();
  }
*/
  
  UCTNode::UCT_NODE_ID UCTNode::findTreeRootToReuse(const std::vector<Common::Point> &old_to_reuse_root_sequence, bool doReleaseUnreusedNodes, bool releaseAsync) {
    assert(m_boardId!= INVALID_NODE_ID);
    assert(m_alloc);

    UCT_NODE_ID current = m_boardId;

    for (vector<Common::Point>::const_iterator it = old_to_reuse_root_sequence.begin();
         it != old_to_reuse_root_sequence.end(); it++) {

      UCTNode *node = m_alloc->get(current);
      Common::Point nextP = *it;

      assert(node);
      assert(nextP != POINT_NULL);

      // cerr << "NextP = " << nextP << endl;
      // const vector<UCT_NODE_ID> &children = node->getChildren();
      // for (vector<UCT_NODE_ID>::const_iterator it = children.begin();
      //      it != children.end(); it++) {
      //   if (*it == INVALID_NODE_ID) continue;
      //   UCTNode *n = getNode(*it);
      //   cerr << " " << n->getPos() << " count = " << n->getGameCount() << endl;
      // }
      
      UCT_NODE_ID nextId = node->getChild(nextP);
      if (doReleaseUnreusedNodes) {
        // release parent and its descendant except for nextId node
        node->setToInvalidNode();
        node->putChild(nextP, INVALID_NODE_ID); // remove parent-child connection
        //releaseNodeAndDescendant(current, INVALID_NODE_ID);
        node->m_alloc->release(current, true, releaseAsync);
      }

      if (nextId == INVALID_NODE_ID) {
        // the sequence is not listed on the tree
        cerr << "Node is not found for move " << nextP << endl;
        return INVALID_NODE_ID;
      }
      cerr << "Node is found for move " << nextP << endl;
      current = nextId;
    }
    return current;
  }

  void UCTNode::setToInvalidMove() {
    m_pos = Common::POINT_NULL;
  }

  void UCTNode::updateAfterOneGame(double win) {
    m_winCount += win;
    //m_winRate = (m_winRate * m_gameCount + win) / (m_gameCount+1);
    m_gameCount++;
  }

  void UCTNode::updateAfterOneGameForRave(double rave_win) {
    m_raveWinCount += rave_win;
    //m_raveWinRate = (m_raveWinRate * m_raveCount + rave_win) / (m_raveCount+1);
    m_raveCount++;
  }

  //double UCTNode::updateUCB(int total_game_count) {
  //  assert(m_gameCount>0);
  //  assert(total_game_count>0);
  //  m_ucbValue = m_winRate + UCB_PARAMETER_C * sqrt(log(total_game_count)/m_gameCount);
  //  return m_ucbValue;
  //}
}
