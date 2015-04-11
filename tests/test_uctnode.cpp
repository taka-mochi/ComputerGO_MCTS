/*#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include "../AI/UCTNode.h"
#include "../AI/UCTNodeAllocator.h"

#include <ctime>

using namespace std;
using namespace Common;
using namespace AI;


TEST(UCTNode, init) {
  //UCTNode::manualInit();
  UCTNodeAllocator alloc(11*11);
  
  size_t init_capacity = alloc.capacity();
  UCTNode *root = alloc.get(alloc.create(1));
  root->putChild(2, alloc.create(2));
  root->putChild(3, alloc.create(3));
  root->putChild(4, alloc.create(4));
  root->putChild(5, alloc.create(5));
  ASSERT_EQ(static_cast<size_t>(5+alloc.capacity()), init_capacity);
  alloc.releaseAll();
  ASSERT_EQ(static_cast<size_t>(0+alloc.capacity()), init_capacity);
}

TEST(UCTNode, release) {
  UCTNodeAllocator alloc(11*11);

  size_t init_capacity = alloc.capacity();
  UCTNode *root = alloc.get(alloc.create(1));
  UCTNode *child_4; UCTNode::UCT_NODE_ID child_4_id;
  UCTNode::UCT_NODE_ID child_5_id;
  root->putChild(2, alloc.create(2));
  root->putChild(3, alloc.create(3));
  root->putChild(4, (child_4_id = alloc.create(4)));
  root->putChild(5, (child_5_id = alloc.create(5)));
  child_4 = alloc.get(child_4_id);

  child_4->putChild(6, alloc.create(6));
  child_4->putChild(7, alloc.create(7));
  child_4->putChild(8, alloc.create(8));
  child_4->putChild(9, alloc.create(9));

  root->putChild(5, UCTNode::INVALID_NODE_ID);
  EXPECT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(child_5_id, false, false));
  EXPECT_EQ(init_capacity, alloc.capacity()+8);
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, root->getChild(5));
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(child_5_id)->getId());

  root->putChild(4, UCTNode::INVALID_NODE_ID);
  EXPECT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(child_4_id, true, false));
  EXPECT_EQ(init_capacity, alloc.capacity()+3);
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, root->getChild(4));  
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(child_4_id)->getId());  
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(alloc.get(child_4_id)->getChild(6))->getId());  
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(alloc.get(child_4_id)->getChild(7))->getId());  
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(alloc.get(child_4_id)->getChild(8))->getId());  
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(alloc.get(child_4_id)->getChild(9))->getId());  
  EXPECT_NE(UCTNode::INVALID_NODE_ID, root->getChild(3));  
  EXPECT_NE(UCTNode::INVALID_NODE_ID, root->getChild(2));  
}

TEST(UCTNode, treeReuse) {
  UCTNodeAllocator alloc(11*11);

  size_t init_cap = alloc.capacity();
  
  UCTNode *root = alloc.get(alloc.create(1));
  UCTNode *child_4; UCTNode::UCT_NODE_ID child_4_id;

  root->putChild(2, alloc.create(2));
  root->putChild(3, alloc.create(3));
  root->putChild(4, (child_4_id = alloc.create(4)));
  child_4 = alloc.get(child_4_id);

  UCTNode *child_6; UCTNode::UCT_NODE_ID child_6_id;
  child_4->putChild(5, alloc.create(5));
  child_4->putChild(6, (child_6_id = alloc.create(6)));
  child_4->putChild(7, alloc.create(7));
  child_4->putChild(8, alloc.create(8));
  child_6 = alloc.get(child_6_id);

  child_6->putChild(9, alloc.create(9));
  child_6->putChild(10, alloc.create(10));
  child_6->putChild(11, alloc.create(11));
  child_6->putChild(12, alloc.create(12));
  child_6->putChild(13, alloc.create(13));

  alloc.waitAsyncNodeReleasingFinish();

  vector<Common::Point> seq;
  seq.push_back(4); seq.push_back(6);
  UCTNode::UCT_NODE_ID next = root->findTreeRootToReuse(seq, false);
  EXPECT_EQ(next, child_6_id);
  UCTNode *nextNode = alloc.get(next);
  EXPECT_NE(UCTNode::INVALID_NODE_ID, nextNode->getChild(9));
  EXPECT_NE(UCTNode::INVALID_NODE_ID, nextNode->getChild(10));
  EXPECT_NE(UCTNode::INVALID_NODE_ID, nextNode->getChild(11));
  EXPECT_NE(UCTNode::INVALID_NODE_ID, nextNode->getChild(12));
  EXPECT_NE(UCTNode::INVALID_NODE_ID, nextNode->getChild(13));

  seq[0] = 4; seq[1] = 10;
  next = root->findTreeRootToReuse(seq, false);
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, next);

  seq[0] = 4; seq[1] = 6;
  next = root->findTreeRootToReuse(seq);
  EXPECT_EQ(next, child_6_id);
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(child_4_id)->getId());
  EXPECT_EQ(UCTNode::INVALID_NODE_ID, alloc.get(child_4_id)->getChild(6));

  alloc.startAsyncNodeReleasing();
  sleep(1);
  alloc.waitAsyncNodeReleasingFinish();
  EXPECT_EQ(init_cap, alloc.capacity()+6);
}

void create_children_80(UCTNode::UCT_NODE_ID parent, int depth, UCTNodeAllocator &alloc) {
  if (depth == 0) return;

  UCTNode *n = alloc.get(parent);
  for (int i=80; i<100; i++) {
    n->putChild(i, UCTNode::INVALID_NODE_ID);
  }
  for (int i=0; i<80; i++) {
    UCTNode::UCT_NODE_ID c = alloc.create(i);
    n->putChild(i,c);
    create_children_80(c, depth-1, alloc);
  }
}

TEST(UCTNode, allocatorCorrect) {
  AI::UCTNodeAllocator alloc(11*11);

  int first_cap = alloc.capacity();

  AI::UCTNode::UCT_NODE_ID root1 = UCTNode::INVALID_NODE_ID, root2 = UCTNode::INVALID_NODE_ID;

  for (int i=0; i<10; i++) {
    cout << "--------- start " << (i+1) << "th UCT search -----------" << endl;
    cout.flush();

    int init_cap = alloc.capacity();

    alloc.waitAsyncNodeReleasingFinish();
    if (root1 != UCTNode::INVALID_NODE_ID) {
      ASSERT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(root1, true, true));
    }
    if (root2 != UCTNode::INVALID_NODE_ID) {
      ASSERT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(root2, true, true));
    }
    alloc.startAsyncNodeReleasing();

    int after_wait_capacity = alloc.capacity();

    // dummy UCT search
    int new1 = alloc.create(0);
    create_children_80(new1, 2, alloc);
    ASSERT_NE(UCTNode::INVALID_NODE_ID, new1);
    int new2 = alloc.create(0);
    create_children_80(new2, 2, alloc);
    ASSERT_NE(UCTNode::INVALID_NODE_ID, new2);
    sleep(1);

    root1 = new1;
    root2 = new2;

    int after_uct_cap = alloc.capacity();

    cout << "Init capacity = " << init_cap << endl <<
      "After Wait Capacity " << after_wait_capacity << endl <<
      "After UCT Capacity " << after_uct_cap << endl;

  }

  alloc.waitAsyncNodeReleasingFinish();
  alloc.release(root1, true);
  alloc.release(root2, true);
  alloc.startAsyncNodeReleasing();
  sleep(1);
  alloc.waitAsyncNodeReleasingFinish();


  ASSERT_EQ(static_cast<size_t>(0), alloc.capacity()%first_cap);
}

TEST(UCTNode, allocator_sync_release) {
  AI::UCTNodeAllocator alloc(11*11);

  size_t init_cap = alloc.capacity();

  UCTNode::UCT_NODE_ID id = alloc.create(0);
  EXPECT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(id, false, false));
  ASSERT_EQ(init_cap, alloc.capacity());

  id = alloc.create(0);
  create_children_80(id, 2, alloc);
  EXPECT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(id, true, false));
  EXPECT_EQ(init_cap, alloc.capacity());
}

TEST(UCTNode, allocator_lockless_release_fail) {
  AI::UCTNodeAllocator alloc(11*11);

  //int init_cap = alloc.capacity();

  UCTNode::UCT_NODE_ID id = alloc.create(0);

  create_children_80(id, 2, alloc);

  //alloc.waitAsyncNodeReleasingFinish();
  ASSERT_EQ(UCTNodeAllocator::RELEASE_NOT_LOCKED, alloc.release(id, true, true));
  EXPECT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(id, true, false));
}

TEST(UCTNode, allocator_startless_release_fail) {
  AI::UCTNodeAllocator alloc(11*11);

  size_t init_cap = alloc.capacity();

  UCTNode::UCT_NODE_ID id = alloc.create(0);

  create_children_80(id, 2, alloc);

  alloc.waitAsyncNodeReleasingFinish();
  ASSERT_EQ(UCTNodeAllocator::RELEASE_OK, alloc.release(id, true, true));
  //alloc.startAsyncNodeReleasing();
  //alloc.waitAsyncNodeReleasingFinish();

  EXPECT_NE(init_cap, alloc.capacity());
}
*/
