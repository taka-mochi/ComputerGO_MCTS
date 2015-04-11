
#include "precomp.h"

#include <iostream>
#include <time.h>
#include <pthread.h>
#include "AI/UCTNodeAllocator.h"

using namespace std;

namespace AI {

  size_t UCTNodeAllocator::ALLOCATION_EACH_COUNT = 10000;

  UCTNodeAllocator::UCTNodeAllocator(size_t maxBoardSize)
    : m_maxBoardSize(maxBoardSize)
  {
    initThread();
    allocateNewNodes();
  }

  UCTNodeAllocator::~UCTNodeAllocator() {
    destroyThread();
    cerr << "------------------------- node size = " << m_allNodes.size() << "--------------------- end of destructor at UCTNodeAllocator " << endl;
    for (size_t i=0; i<m_allNodes.size(); i++) {
      delete m_allNodes[i];
    }
    m_allNodes.clear();
    m_allNodes = vector<UCTNode *>();
    m_usable.clear();
    m_usable = vector<UCTNode::UCT_NODE_ID>();
  }

  UCTNode::UCT_NODE_ID UCTNodeAllocator::create(Common::Point p) {
    if (m_usable.size() == 0) {
      cerr << "UCT Node reallocation start" << endl;
      allocateNewNodes();
      cerr << "UCT Node reallocation finished" << endl;
    } 

    UCTNode::UCT_NODE_ID new_id = m_usable.back();
    m_usable.pop_back();
    UCTNode *newnode = m_allNodes[new_id-1];
    newnode->init(this, m_maxBoardSize, new_id, p);

    return new_id;
  }

  void UCTNodeAllocator::initThread() {
    m_shouldDie = false;
    pthread_mutex_init(&m_releaseBufferMutex, NULL);
    pthread_cond_init(&m_releaseStartSignal, NULL);

    pthread_create(&m_enumerateReleaseThread, NULL, UCTNodeAllocator::releaseThreadEntryPoint, reinterpret_cast<void *>(this));

    m_isToBeReleasedLocked = false;
  }

  void UCTNodeAllocator::destroyThread() {
    m_shouldDie = true;
    for(int i=0; i<10; i++) startAsyncNodeReleasing();
    void *p;
    cerr << "wait to finish thread..." << endl;
    timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 90; // wait max 1.5min
    pthread_timedjoin_np(m_enumerateReleaseThread, &p, &timeout);
    cerr << "thread finishes..." << endl;

    pthread_mutex_destroy(&m_releaseBufferMutex);
    pthread_cond_destroy(&m_releaseStartSignal);
  }

  // 
  // Allocation when needed
  //
  void UCTNodeAllocator::allocateNewNodes() {
    cerr << "New Allocated!!" << endl;
    cerr.flush();
    m_allNodes.reserve(m_allNodes.size()+ALLOCATION_EACH_COUNT);
    m_usable.reserve(m_usable.size()+ALLOCATION_EACH_COUNT);
    for (size_t i=0; i<ALLOCATION_EACH_COUNT; i++) {
      UCTNode *a = new UCTNode;//(this, m_maxBoardSize);
      m_allNodes.push_back(a);
      m_usable.push_back(m_allNodes.size());
    }
    // for (size_t i=0; i<m_allNodes.size(); i++) {
    //   delete m_allNodes[i];
    // }
    // m_allNodes.clear();
    // m_usable.clear();
    cerr << "All Node Size = " << m_allNodes.size() << " Allocated Size = " << sizeof(UCTNode)*ALLOCATION_EACH_COUNT << endl;
  }

  UCTNodeAllocator::ReleaseErrorType UCTNodeAllocator::release(UCTNode::UCT_NODE_ID id, bool isRecursive, bool asynchronize) {
    //cerr << "Release is called" << endl;
    //cerr.flush();
    if (asynchronize) {
      if (!m_isToBeReleasedLocked) {
        return RELEASE_NOT_LOCKED;
      }
      m_toBeReleased.push_back(ReleaseNodeInfo(id, isRecursive));
    } else {
      if (!isRecursive) {
        UCTNode *node = get(id);
        if (node) {
          node->setToInvalidNode();
          m_usable.push_back(id);
        }
      } else {
        ///////////////////////////////////////////
        release_recursive_directly(id);
      }
    }

    return RELEASE_OK;
  }

  void UCTNodeAllocator::releaseAll() {
    bool wasLocked = m_isToBeReleasedLocked;
    if (!m_isToBeReleasedLocked) {
      waitAsyncNodeReleasingFinish();
    }
    m_usable.clear();
    m_readyToUse.clear();
    for (size_t i=0; i<m_allNodes.size(); i++) {
      m_usable.push_back(i);
    }
    if (!wasLocked) {
      startAsyncNodeReleasing();
    }
  }

  // 
  // Interface to stop and start release thread
  //
  void UCTNodeAllocator::waitAsyncNodeReleasingFinish() {
    pthread_mutex_lock(&m_releaseBufferMutex);
    m_isToBeReleasedLocked = true;
    //cerr << "Got lock" << endl;
    //cerr.flush();
    release_internal_by_readyToUseList(); // merge readyToUseList
  }

  void UCTNodeAllocator::startAsyncNodeReleasing() {
    pthread_cond_signal(&m_releaseStartSignal);
    m_isToBeReleasedLocked = false;
    cerr << "Send signal" << endl;
    cerr.flush();
    pthread_mutex_unlock(&m_releaseBufferMutex);
    cerr << "Unlock" << endl;
    cerr.flush();
  }

  // 
  // Release Utility
  //
  void UCTNodeAllocator::release_recursive_directly(UCTNode::UCT_NODE_ID id) {
    std::vector<UCTNode::UCT_NODE_ID> l;
    enumerate_recursive_internal(id, l);
    release_internal_by_list(l);
  }

  void UCTNodeAllocator::release_internal_by_readyToUseList() {
    release_internal_by_list(m_readyToUse);
    cerr << "Ready To Use Size: " << m_readyToUse.size() << endl;
    m_readyToUse.resize(0);
  }

  void UCTNodeAllocator::release_internal_by_list(const std::vector<UCTNode::UCT_NODE_ID> &l) {
    for (std::vector<UCTNode::UCT_NODE_ID>::const_iterator it = l.begin();
         it != l.end(); it++) {
      get(*it)->setToInvalidNode();
      m_usable.push_back(*it);
    }
  }
  
  void UCTNodeAllocator::enumerate_recursive_internal(UCTNode::UCT_NODE_ID id, std::vector<UCTNode::UCT_NODE_ID> &enumerated) {
    UCTNode *node = get(id);
    if (!node) return;

    node->setToInvalidNode();
    vector<UCTNode::UCT_NODE_ID> children = node->getChildren();
    for (vector<UCTNode::UCT_NODE_ID>::iterator it = children.begin();
       it != children.end(); it++) {
      if (*it != UCTNode::INVALID_NODE_ID) {
        enumerate_recursive_internal(*it, enumerated);
      }
    }
    enumerated.push_back(id);
  }

  //
  // Release Thread
  // 
  void *UCTNodeAllocator::releaseThreadEntryPoint(void *p) {
    reinterpret_cast<UCTNodeAllocator *>(p)->releaseThreadMainLoop();
    return NULL;
  }

  void UCTNodeAllocator::releaseThreadMainLoop() {
    while (!m_shouldDie) {
#ifdef DEBUG
      cerr << "Release Thread: Waiting..." << endl;
#endif
      pthread_mutex_lock(&m_releaseBufferMutex);
      pthread_cond_wait(&m_releaseStartSignal, &m_releaseBufferMutex);

      // hogehoge
#ifdef DEBUG
      cerr << "Release Thread: Start to Release Thread!!" << endl;
      cerr.flush();
#endif

      vector<ReleaseNodeInfo>::iterator it = m_toBeReleased.begin();
      for (; it != m_toBeReleased.end(); it++) {
        if (it->isRecursive) {
          enumerate_recursive_internal(it->id, m_readyToUse);
        } else {
          m_readyToUse.push_back(it->id);
        }
      }
      m_toBeReleased.clear();
#ifdef DEBUG
      cerr << "Release Thread: Finish to collect release list!!" << endl;
      cerr.flush();
#endif
      pthread_mutex_unlock(&m_releaseBufferMutex);
    }
  }
}
