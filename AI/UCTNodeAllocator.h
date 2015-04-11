
#pragma once

#include <pthread.h>

#include "common.h"
#include "AI/UCTNode.h"

#include <cstring>

namespace AI {
  class UCTNodeAllocator {
    size_t m_maxBoardSize;

    std::vector<UCTNode *> m_allNodes;
    std::vector<UCTNode::UCT_NODE_ID> m_usable;
    std::vector<UCTNode::UCT_NODE_ID> m_readyToUse;
    struct ReleaseNodeInfo {
      UCTNode::UCT_NODE_ID id;
      bool isRecursive;
      ReleaseNodeInfo(UCTNode::UCT_NODE_ID _id = UCTNode::INVALID_NODE_ID, bool _isRecursive = true)
        : id(_id)
        , isRecursive(_isRecursive)
      {}
    };
    std::vector<ReleaseNodeInfo> m_toBeReleased;

    // thread related data
    bool m_shouldDie;
    bool m_isToBeReleasedLocked;
    pthread_t m_enumerateReleaseThread;
    pthread_mutex_t m_releaseBufferMutex;
    pthread_cond_t m_releaseStartSignal;

    static size_t ALLOCATION_EACH_COUNT;

  public:
    explicit UCTNodeAllocator(size_t maxBoardSize);
    ~UCTNodeAllocator();

    UCTNode::UCT_NODE_ID create(Common::Point p);
    inline UCTNode *get(UCTNode::UCT_NODE_ID id) {
#ifdef DEBUG
      if (id < 1 || id > (signed)m_allNodes.size()) return NULL;
      return m_allNodes.at(id-1);
#else
      if (id == UCTNode::INVALID_NODE_ID) return NULL;
      return m_allNodes[id-1];
#endif
    }
    inline const UCTNode *get(UCTNode::UCT_NODE_ID id) const {
#ifdef DEBUG
      if (id < 1 || id > (signed)m_allNodes.size()) return NULL;
      return m_allNodes.at(id-1);
#else
      if (id == UCTNode::INVALID_NODE_ID) return NULL;
      return m_allNodes[id-1];
#endif
    }
    inline size_t capacity() const {return m_usable.size();}

    enum ReleaseErrorType {
      RELEASE_OK, RELEASE_NOT_LOCKED
    };

    ReleaseErrorType release(UCTNode::UCT_NODE_ID id, bool isRecursive, bool asynchronize = true);
    void releaseAll();

    void waitAsyncNodeReleasingFinish();
    void startAsyncNodeReleasing();

  private:
    void allocateNewNodes();
    void initThread();
    void destroyThread();

    // release utility
    void release_recursive_directly(UCTNode::UCT_NODE_ID id);
    void release_internal_by_readyToUseList();
    void release_internal_by_list(const std::vector<UCTNode::UCT_NODE_ID> &list);
    void enumerate_recursive_internal(UCTNode::UCT_NODE_ID id, std::vector<UCTNode::UCT_NODE_ID> &enumerated);

    // Thread Related Methods
    static void *releaseThreadEntryPoint(void *p);
    void releaseThreadMainLoop();
  };
}
