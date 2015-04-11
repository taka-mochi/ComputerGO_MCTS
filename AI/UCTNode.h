
#pragma once

#include "common.h"
#include <vector>
#include <cstring>

namespace AI {
  class UCTInternal;
  class UCTNodeAllocator;
  
  class UCTNode {
  public:
    typedef int UCT_NODE_ID;
    typedef std::vector<UCT_NODE_ID> UctNodeIdList;
    static const UCT_NODE_ID INVALID_NODE_ID;

    friend class UCTNodeAllocator;

  private:
    friend class UCTInternal;

    UCTNodeAllocator *m_alloc;

    // should have hash value?
    UCT_NODE_ID m_boardId;
    Common::Point m_pos;
    int m_gameCount;  // selected count for this node (include myself playout)
    double m_winCount;
    int m_raveCount;
    int m_raveChildrenTotalCount;
    double m_raveWinCount;

    bool m_areChildrenExpanded;

    //double m_ucbValue;
    std::vector<UCT_NODE_ID> m_children, m_insertedChildren;
    //int m_childrenCount;
  public:
    explicit UCTNode();
    explicit UCTNode(UCTNodeAllocator *alloc, size_t maxBoardSize);
    UCTNode(const UCTNode &r) : m_alloc(r.m_alloc) {}
    UCTNode &operator =(const UCTNode &r) {return *this;}
    ~UCTNode();
  private:

    void init(UCT_NODE_ID boardId, Common::Point pos);
    void init(UCTNodeAllocator *alloc, size_t maxBoardSize, UCT_NODE_ID boardId, Common::Point pos);

    //static void releaseNodeAndDescendant_internal(UCTNode::UCT_NODE_ID id);
  public:
    //static void manualInit();
    //static UCT_NODE_ID createNewNode(Common::Point pos);
    //static UCTNode *getNode(UCT_NODE_ID id);
    //static void releaseAllNode();
    //static void releaseNode(UCT_NODE_ID id, UCT_NODE_ID parentID);
    //static void releaseNodeAndDescendant(UCT_NODE_ID id, UCT_NODE_ID parentID);
    //static size_t getNewNodeCapacity();

    UCT_NODE_ID findTreeRootToReuse(const std::vector<Common::Point> &old_to_reuse_root_sequence, bool doReleaseUnreusedNodes = true, bool releaseAsync = true);

    void updateAfterOneGame(double win);
    void updateAfterOneGameForRave(double rave_win);
    void incrementChildRaveCount(){m_raveChildrenTotalCount++;}

    inline void setToInvalidNode() {m_boardId = INVALID_NODE_ID;}
    inline UCT_NODE_ID getId() const {return m_boardId;}
    void setToInvalidMove();
    inline void setToChildrenAreExpanded() {m_areChildrenExpanded = true;}
    inline bool areChildrenExpanded() const {return m_areChildrenExpanded;}
    inline Common::Point getPos() const {return m_pos;}
    inline int getGameCount() const {return m_gameCount;}
    inline double getWinCount() const {return m_winCount;}
    inline int getRaveCount() const {return m_raveCount;}
    inline int getChildrenTotalRaveCount() const {return m_raveChildrenTotalCount;}
    inline double getRaveWinCount() const {return m_raveWinCount;}
    //double updateUCB(int total_game_count);
    inline const std::vector<UCT_NODE_ID> &getChildren() const {return m_insertedChildren;}//{return m_children;}
    inline UCT_NODE_ID getChild(Common::Point p) const {return m_children[p+1];}
    //inline int getChildrenCount() const {return m_childrenCount;}
    inline void putChild(Common::Point p, UCT_NODE_ID id) {
      //if (m_children[p+1] == INVALID_NODE_ID) {
        // first put
      if (id != INVALID_NODE_ID) {
        m_insertedChildren.push_back(id);
      } else {
        std::vector<UCT_NODE_ID>::iterator it = std::find(m_insertedChildren.begin(), m_insertedChildren.end(), m_children[p+1]);
        if (it != m_insertedChildren.end()) {
          m_insertedChildren.erase(it);
        }
      }
      m_children[p+1] = id;
    }
  };
}
