
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <pthread.h>

using namespace std;

struct Node {
  int value;
  vector<int> children;
  Node()
    : children(100,-1)
  {
  }
};

class NodeAllocator {
  vector<Node *> m_allNodes;
  vector<int> m_usable;
  vector<int> m_readyToUse;
  struct ReleaseNodeInfo {
    bool isRecursive;
    int nodeId;
  };
  vector<ReleaseNodeInfo> m_toBeReleased;

  bool m_isToBeReleasedLocked;
  pthread_t m_enumerateReleaseThread;
  pthread_mutex_t m_releaseBufferMutex;
  pthread_cond_t m_releaseStartSignal;

  void allocateNewNodes() {
    cout << "New Allocated!!" << endl;
    cout.flush();
    for (int i=0; i<300000; i++) {
      m_allNodes.push_back(new Node);
      m_usable.push_back(m_allNodes.size()-1);
    }
    cout << "All Node Size = " << m_allNodes.size() << endl;
  }

  
  void initThread() {
    pthread_mutex_init(&m_releaseBufferMutex, NULL);
    pthread_cond_init(&m_releaseStartSignal, NULL);

    pthread_create(&m_enumerateReleaseThread, NULL, &NodeAllocator::releaseThreadEntryPoint, reinterpret_cast<void *>(this));

    m_isToBeReleasedLocked = false;
  }

public:
  NodeAllocator()
    : m_allNodes()
    , m_usable()
    , m_readyToUse()
    , m_toBeReleased()
  {
    allocateNewNodes();
    initThread();
  }

  int create() {
    if (m_usable.size() == 0) {
      allocateNewNodes();
    }
    int n = m_usable.back();
    m_usable.pop_back();
    return n;
  }
  Node *get(int n) {
    return m_allNodes[n];
  }

  int capacity() {
    return m_usable.size();
  }

  enum ReleaseErrorType {
    RELEASE_OK, RELEASE_NOT_LOCKED
  };

  ReleaseErrorType release(int id) {
    if (m_isToBeReleasedLocked) {
      cerr << "Release Method is Called!!" << endl;
      ReleaseNodeInfo info;
      info.nodeId = id;
      info.isRecursive = false;
      m_toBeReleased.push_back(info);
    } else {
      return RELEASE_NOT_LOCKED;
    }
    return RELEASE_OK;
  }
  ReleaseErrorType release_recursively(int id) {
    if (m_isToBeReleasedLocked) {
      cerr << "Release Recursive Method is Called!!" << endl;
      ReleaseNodeInfo info;
      info.nodeId = id;
      info.isRecursive = true;
      m_toBeReleased.push_back(info);
    } else {
      return RELEASE_NOT_LOCKED;
    }
    return RELEASE_OK;
  }

  void waitAsyncNodeReleaseFinish() {
    pthread_mutex_lock(&m_releaseBufferMutex);
    m_isToBeReleasedLocked = true;
    cerr << "Got lock" << endl;
    cerr.flush();
    release_internal_by_readyToUseList(); // merge readyToUseList
  }

  void startAsyncNodeRelease() {
    pthread_cond_signal(&m_releaseStartSignal);
    m_isToBeReleasedLocked = false;
    cerr << "Send signal" << endl;
    cerr.flush();
    pthread_mutex_unlock(&m_releaseBufferMutex);
    cerr << "Unlock" << endl;
    cerr.flush();
  }



private:

  void release_internal_by_readyToUseList() {
    release_internal_by_list(m_readyToUse);
    cout << "size of ready to use:" << m_readyToUse.size() << endl;
    m_readyToUse.clear();
  }

  void release_internal_by_list(vector<int> &l) {
    for (vector<int>::iterator it = l.begin();
         it != l.end(); it++ ) {
      m_usable.push_back(*it);
    }
  }

  void enumerate_recursive_internal(int id, vector<int> &enumerated) {
    Node *n = get(id);
    for (vector<int>::iterator it = n->children.begin();
       it != n->children.end(); it++) {
      int c = *it;
      if (c !=-1) {
        enumerate_recursive_internal(c, enumerated);
      }
    }
    enumerated.push_back(id);
    
  }

  static void *releaseThreadEntryPoint(void *p) {
    NodeAllocator *al = reinterpret_cast<NodeAllocator *>(p);
    al->releaseThreadMainLoop();
    return NULL;
  }

  void releaseThreadMainLoop() {
    while (true) {
      cerr << "Thread: Waiting..." << endl;
      cerr.flush();
      pthread_mutex_lock(&m_releaseBufferMutex);
      pthread_cond_wait(&m_releaseStartSignal, &m_releaseBufferMutex);
      cerr << "Thread: Start to Release Thread!!" << endl;
      cerr.flush();

      vector<ReleaseNodeInfo>::iterator it = m_toBeReleased.begin();
      for (; it != m_toBeReleased.end(); it++) {
        if (it->isRecursive) {
          enumerate_recursive_internal(it->nodeId, m_readyToUse);
        } else {
          m_readyToUse.push_back(it->nodeId);
        }
      }
      m_toBeReleased.clear();

      cerr << "Thread: Finish to collect release list!!" << endl;
      pthread_mutex_unlock(&m_releaseBufferMutex);
    }
  }

};


void create_children_80(int parent, int depth, NodeAllocator &alloc) {
  if (depth == 0) return;

  Node *n = alloc.get(parent);
  for (int i=80; i<100; i++) {
    n->children[i] = -1;
  }
  for (int i=0; i<80; i++) {
    int c = alloc.create();
    n->children[i] = c;
    create_children_80(c, depth-1, alloc);
  }
}

double getmsec(timeval &t) {
  return 1000.0*t.tv_sec + (double)t.tv_usec * 1e-3;
}

double timediffinmsec(timeval &begin, timeval &end) {
  return getmsec(end) - getmsec(begin);
}

int main() {

  NodeAllocator alloc;

  timeval b,e;

  int to_release_root = -1;
  int to_release_root2 = -1;
  int to_release_root3 = -1;


  for (int i=0; i<10; i++) {
    cout << "--------- start " << (i+1) << "th UCT search -----------" << endl;
    cout.flush();

    int init_cap = alloc.capacity();
    
    gettimeofday(&b, NULL);
    alloc.waitAsyncNodeReleaseFinish();
    if (to_release_root != -1) {
      if (alloc.release_recursively(to_release_root) == NodeAllocator::RELEASE_NOT_LOCKED) {
        cout << "You should execute waitAsync..." << endl;
      }
    }
    if (to_release_root2 != -1) {
      if (alloc.release_recursively(to_release_root2) == NodeAllocator::RELEASE_NOT_LOCKED) {
        cout << "You should execute waitAsync..." << endl;
      }
    }
    if (to_release_root3 != -1) {
      if (alloc.release_recursively(to_release_root3) == NodeAllocator::RELEASE_NOT_LOCKED) {
        cout << "You should execute waitAsync..." << endl;
      }
    }
    alloc.startAsyncNodeRelease();

    int after_wait_capacity = alloc.capacity();

    gettimeofday(&e, NULL);
    cout << "Time (msec) for sync and release children  = " << timediffinmsec(b,e) << endl;
    cout.flush();
  
    cout << "Executing UCT search..." << endl;
    cout.flush();
    int root = alloc.create();
    create_children_80(root, 3, alloc);
    int root2 = alloc.create();
    create_children_80(root2, 3, alloc);
    int root3 = alloc.create();
    create_children_80(root3, 3, alloc);
    sleep(1);
    cout << "End UCT search..." << endl;
    cout.flush();

    to_release_root = root;
    to_release_root2 = root2;
    to_release_root3 = root3;

    int after_uct_cap = alloc.capacity();

    cout << "Init capacity = " << init_cap << endl <<
      "After Wait Capacity " << after_wait_capacity << endl <<
      "After UCT Capacity " << after_uct_cap << endl;
  }

  alloc.waitAsyncNodeReleaseFinish();
  alloc.release_recursively(to_release_root);
  alloc.release_recursively(to_release_root2);
  alloc.startAsyncNodeRelease();
  sleep(1);
  alloc.waitAsyncNodeReleaseFinish();
  cout << "All After Capacity " << alloc.capacity() << endl;

  return 0;
}
