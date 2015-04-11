#include "precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>

#include "ML/FeatureArray.h"
#include "ML/LightFeatureExtractor_ForTest.h"
#include "ML/MM_Learning.h"

using namespace std;
using namespace Common;
using namespace ML;
using namespace Go;

class MM_Learning_Test : public ::testing::Test {
protected:
  virtual void SetUp() {
    shared_ptr<SparseVector> test_feature_1(new SparseVector(7));
    shared_ptr<SparseVector> test_feature_2(new SparseVector(7));
    shared_ptr<SparseVector> test_feature_3(new SparseVector(7));

    (*test_feature_1).add(0); (*test_feature_1).add(1); (*test_feature_1).add(2);
    (*test_feature_2).add(1); (*test_feature_2).add(3);
    (*test_feature_3).add(0); (*test_feature_3).add(4); (*test_feature_3).add(5); (*test_feature_3).add(6);

    vector<shared_ptr<SparseVector> > test_set_1, test_set_2, test_set_3;
    test_set_1.push_back(test_feature_1);
    test_set_1.push_back(test_feature_2);
    test_set_1.push_back(test_feature_3);
    test_feature_set.push_back(test_set_1);

    test_feature_1.reset(new SparseVector(7));
    test_feature_2.reset(new SparseVector(7));
    (*test_feature_1).add(0);
    (*test_feature_2).add(1);
    test_set_2.push_back(test_feature_1);
    test_set_2.push_back(test_feature_2);
    test_feature_set.push_back(test_set_2);

    test_feature_1.reset(new SparseVector(7));
    test_feature_2.reset(new SparseVector(7));
    (*test_feature_1).add(1); (*test_feature_1).add(4);
    (*test_feature_2).add(3);
    test_set_3.push_back(test_feature_1);
    test_set_3.push_back(test_feature_2);
    test_feature_set.push_back(test_set_3);
  }
  virtual void TearDown() {
  }

  void test_CalcProdOfFeaturesWeights() {
    trainer.initWeights();
    for (size_t i=0; i<extractor.getFeatureDimension(); i++) {
      trainer.m_featureWeights[i] = i+1;
    }

    //FeatureValues testFeatures(extractor.getFeatureDimension());
    SparseVector testFeatures(extractor.getFeatureDimension());
    {
      // test case 1
      testFeatures.clear();
      double expect_val = 1;
      for (size_t i=0; i<extractor.getFeatureDimension(); i+=2) {
        testFeatures.add(i);
        //testFeatures[i] = 1;
        expect_val *= (i+1);
      }
      double val = trainer.calculateProductOfExistFeatureWeights(testFeatures);
      EXPECT_TRUE(fabs(val-expect_val) < 0.0001);
    }

    {
      // test case 2
      testFeatures.clear();
      double expect_val = 1;
      double val = trainer.calculateProductOfExistFeatureWeights(testFeatures);
      EXPECT_TRUE(fabs(val-expect_val) < 0.0001);
    }

    {
      // test case 3
      for (size_t i=0; i<extractor.getFeatureDimension(); i++) {
        testFeatures.add(i);
        //testFeatures[i] = 1;
      }
      double expect_val = 1;
      for (size_t i=0; i<extractor.getFeatureDimension(); i++) {
        expect_val *= (i+1);
      }
      //cerr << testFeatures.toString() << endl;
      double val = trainer.calculateProductOfExistFeatureWeights(testFeatures);
      EXPECT_TRUE(fabs(val-expect_val) < 0.0001);
    }

    {
      // test case 4
      testFeatures.clear();
      testFeatures.add(0); testFeatures.add(1);
      //testFeatures[0] = testFeatures[1] = 1;
      double expect_val = 1*2;
      double val = trainer.calculateProductOfExistFeatureWeights(testFeatures);
      EXPECT_TRUE(fabs(val-expect_val) < 0.0001);
    }
  }

  void test_CalcProdOfTeammateWeights() {
    trainer.initWeights();
    for (size_t i=0; i<extractor.getFeatureDimension(); i++) {
      trainer.m_featureWeights[i] = (i+1)*(i+1);
      trainer.m_featureWeights_inverse[i] = 1.0/trainer.m_featureWeights[i];
    }

    SparseVector testFeatures(extractor.getFeatureDimension());
    {
      // test case 1
      testFeatures.clear();
      double expect_val = 1;
      for (size_t i=0; i<extractor.getFeatureDimension(); i+=2) {
        testFeatures.add(i);
        //testFeatures[i] = 1;
      }
      double prod = trainer.calculateProductOfExistFeatureWeights(testFeatures);
      for (size_t i=0; i<extractor.getFeatureDimension(); i+=2) {
        double val = trainer.calculateProductOfTeammateWeights(testFeatures, i, prod);
        expect_val = 1;
        for (size_t j=0; j<extractor.getFeatureDimension(); j+=2) {
          if (j!=i) expect_val *= (j+1)*(j+1);
        }
        EXPECT_TRUE(fabs(val-expect_val) < 0.0001);
      }
    }

    {
      // test case 2
      testFeatures.clear();
      for (size_t i=0; i<extractor.getFeatureDimension(); i++) {
        //testFeatures[i] = 1;
        testFeatures.add(i);
      }
      double expect_val = 1;
      double prod = trainer.calculateProductOfExistFeatureWeights(testFeatures);
      for (size_t i=0; i<extractor.getFeatureDimension(); i++) {
        double val = trainer.calculateProductOfTeammateWeights(testFeatures, i, prod);
        expect_val = 1;
        for (size_t j=0; j<extractor.getFeatureDimension(); j++) {
          if (j!=i) expect_val *= (j+1)*(j+1);
        }
        EXPECT_TRUE(fabs(val-expect_val) < 0.0001);
      }
    }

    {
      // test case 3
      testFeatures.clear();
      testFeatures.add(0); testFeatures.add(1);
      //testFeatures[0] = testFeatures[1] = 1;

      double prod = trainer.calculateProductOfExistFeatureWeights(testFeatures);
      double val = trainer.calculateProductOfTeammateWeights(testFeatures, 0, prod);
      double expect_val = 2*2;
      EXPECT_TRUE(fabs(val-expect_val) < 0.0001);

      val = trainer.calculateProductOfTeammateWeights(testFeatures, 1, prod);
      expect_val = 1*1;
      EXPECT_TRUE(fabs(val-expect_val) < 0.0001);
    }

  }

  void test_updateForVectorsOfOne() {
    DummyFeatureExtractor_size7 ex;
    MM_Learning trainer(ex);

    vector<shared_ptr<SparseVector> > dataset1(test_feature_set.at(0));
    vector<shared_ptr<SparseVector> > dataset2(test_feature_set.at(0));
    vector<shared_ptr<SparseVector> > dataset3(test_feature_set.at(0));

    trainer.initWeights();

    double expected_CijofEj_values[7][3] = {
      {2.0/3, 1.0/2, 0.0},
      {19.0/31, 7.0/19, 1.0/2},
      {0.43030459, 0, 0}
    };
    int expected_appear_counts[7][3] = {
      {1, 1, 0},
      {1, 0, 1},
      {1, 0, 0}
    };
    double expected_new_weights[7] = {
      12.0/7, 1.350143266, 2.323935229
    };

    double total_CijofEj = 0.0;
    int total_appear_count = 0;
    double CijofEj = 0.0;
    int feature_appear_count_in_training = 0;

    // iter 0
    for (int i=0; i<3; i++) {
      // dim i (max dim is 7, but test 3 or less)
      total_appear_count = 0;
      total_CijofEj = 0;

      for (int j=0; j<(signed)test_feature_set.size(); j++) {
        vector<SparseVector *> dataset;
        for (size_t k=0; k<test_feature_set.at(j).size(); k++) {
          dataset.push_back(test_feature_set.at(j).at(k).get());
        }

        // dataset j
        trainer.updateForFeatureVectorsOfOneData(dataset, 0, i, CijofEj, feature_appear_count_in_training);
        EXPECT_LT(fabs(CijofEj - expected_CijofEj_values[i][j]), 0.0001);  // check CijofEj
        EXPECT_EQ(expected_appear_counts[i][j], feature_appear_count_in_training); // check appear_count
        total_appear_count += feature_appear_count_in_training;
        total_CijofEj += CijofEj;
      }

      trainer.m_featureWeights[i] = total_appear_count / total_CijofEj; // update
      EXPECT_LT(fabs(trainer.m_featureWeights[i] - expected_new_weights[i]), 0.001);
    }    
  }

  void test_convertOneDataToFeatureVectors_in_Eye() {
    Board_Move_Dataset_vector dataset;
    {
      int init_array[] = {
        3,3,3,3,3,3,3,
        3,2,2,2,1,0,3,
        3,2,2,2,1,1,3,
        3,2,2,1,0,0,3,
        3,2,2,2,1,1,3,
        3,0,2,1,0,0,3,
        3,3,3,3,3,3,3
      };
      shared_ptr<Board> board(new Board(5, init_array));
      Point train_move = board->xyToPoint(0,4);
      dataset.addData(board, train_move, 1, POINT_NULL);
      std::vector<SparseVector *> features;
      Point buffer[MAX_BOARD_SIZE+1]; int index;
      GoLearning::convertOneDataToFeatureVectors(trainer.m_featureExtractor, dataset.get(), buffer, features, index);
      EXPECT_EQ(7, (signed)features.size());
      MM_Learning::releaseSparseVectors(features);
    }
  }

  // TEST_F(MM_Learning_Test, convertDataToLightFeatureVectors)
  void test_convertDataToLightFeatureVectors() {
    Point moves_buf[MAX_BOARD_SIZE+1];

    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,1,1,0,0,0,0,3,
      3,0,0,1,2,2,1,0,0,0,3,
      3,0,0,0,0,1,0,0,0,0,3,
      3,0,0,0,0,1,2,2,0,0,3,
      3,0,0,0,0,2,1,2,0,0,3,
      3,0,0,0,0,1,1,2,0,0,3,
      3,0,0,0,0,2,2,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    shared_ptr<Board> board(new Board(9, init_array));
    Color turn = 1;

    Board_Move_Data data(board, PASS, turn, POINT_NULL);
    Point moves[MAX_BOARD_SIZE+1]; int move_counts = 0;

    board->enumerateFreeMoves(moves, move_counts, turn);

    for (int i=0; i<move_counts; i++) {
      if (board->checkLegalHand(moves[i], turn, Board::flipColor(turn)) != Board::PUT_LEGAL) continue;
        
      // set as a move in data
      data.move = moves[i];

      vector<SparseVector *> converted_features;
      int train_index = 0;
      trainer.convertOneDataToFeatureVectors(trainer.m_featureExtractor, data, moves_buf, converted_features, train_index);

      // in this example, a number of legal hands is equal to a number of free points minus 1 (for PASS)
      EXPECT_EQ(move_counts+1, (signed)converted_features.size());
      // index check
      EXPECT_EQ(i, train_index);
        
      for (int j=0; j<(signed)converted_features.size()-1; j++) {
        int x = board->pointToXY(moves[j]).first;
        int y = board->pointToXY(moves[j]).second;
        int minX = std::min(x, board->getSize()-(x+1));
        int minY = std::min(y, board->getSize()-(y+1));
        int minDist = std::min(minX, minY);
        int should_index = LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0 + minDist;
        if (minDist >= 4) {
          should_index = LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE;
        }
        EXPECT_TRUE(converted_features[j]->contains(should_index));

        if ((x==3 && y==5) ||
            (x==3 && y==3)) {
          EXPECT_TRUE(converted_features[j]->contains(LightFeatureExtractor_ForTest::CAPTURE));
        } else{
          EXPECT_FALSE(converted_features[j]->contains(LightFeatureExtractor_ForTest::CAPTURE));
        }
      }
      MM_Learning::releaseSparseVectors(converted_features);
    }
  }

  // TEST_F(MM_Learning_Test, learnWithDataset)
  void test_learnWithDataset() {
    // 実際のboardで、手で追える範囲で学習を行なう
    // 1: 正しく weights が学習できているか
    // 2: 収束しているか
    // この2つをチェック

    Board_Move_Dataset_vector dataset;
    {
      // data1
      int init_array[] = {
        3,3,3,3,3,3,3,
        3,2,2,2,1,1,3,
        3,2,2,2,1,1,3,
        3,2,2,1,0,0,3,
        3,2,2,2,1,1,3,
        3,0,2,1,0,0,3,
        3,3,3,3,3,3,3
      };
      shared_ptr<Board> board(new Board(5, init_array));
      Point train_move = board->xyToPoint(0,4);
      dataset.addData(board, train_move, 1, POINT_NULL);
      std::vector<SparseVector *> features;
      Point buffer[MAX_BOARD_SIZE+1]; int index;
      GoLearning::convertOneDataToFeatureVectors(trainer.m_featureExtractor, dataset.get(), buffer, features, index);
      EXPECT_EQ(6, (signed)features.size());
      MM_Learning::releaseSparseVectors(features);
      // cout << "train index = " << index << endl;
      // for (int i=0; i<(signed)features.size(); i++) {
      //   cout << features[i]->toString() << endl;
      // }
    }
    {
      // data2
      int init_array[] = {
        3,3,3,3,3,3,3,
        3,1,1,0,2,1,3,
        3,1,2,2,1,1,3,
        3,1,2,0,0,1,3,
        3,1,2,2,1,1,3,
        3,1,1,0,2,1,3,
        3,3,3,3,3,3,3
      };
      shared_ptr<Board> board(new Board(5, init_array));
      Point train_move = board->xyToPoint(2,0);
      dataset.addData(board, train_move, 1, POINT_NULL);

      std::vector<SparseVector *> features;
      Point buffer[MAX_BOARD_SIZE+1]; int index;
      dataset.next();
      GoLearning::convertOneDataToFeatureVectors(trainer.m_featureExtractor, dataset.get(), buffer, features, index);
      EXPECT_EQ(5, (signed)features.size());
      MM_Learning::releaseSparseVectors(features);
      // cout << "train index = " << index << endl;
      // for (int i=0; i<(signed)features.size(); i++) {
      //   cout << features[i]->toString() << endl;
      // }
    }

    //cout << "at once update" << endl;
    {
      double expected_weights_per_iterations[3][6] = {
        {60.0/17, 15.0/8, 0, 0, 0, 0},
        {4.938055, 2.022393, 0, 0, 0, 0},
        {6.42174, 2.13594, 0, 0, 0, 0}
      };

      trainer.initWeights();    
      int m_maxIterationCountForDataSet = 3;
      trainer.setParameters(0);
      for (int count=0; count<m_maxIterationCountForDataSet; count++) {
        FeatureWeights newWeights(6);
        vector<Board_Move_Dataset *> dataset_wrap;
        dataset_wrap.push_back(&dataset);
        trainer.learn_at_once(dataset_wrap);
        newWeights = trainer.m_featureWeights;
        for (size_t i=0; i<trainer.m_featureWeights.size(); i++) {
          //newWeights[i] = trainer.calculateNewOneDimensionOfWeights(dataset, i);
          if (fabs(expected_weights_per_iterations[count][i]-newWeights[i]) > 0.001) {
            cerr << dec << "index = " << i << " expected[i] = " << expected_weights_per_iterations[count][i] << " actual[i] = " << newWeights[i] << endl;
          }
          EXPECT_LE(fabs(expected_weights_per_iterations[count][i]-newWeights[i]), 0.001);

          if (expected_weights_per_iterations[count][i] != 0) {
            EXPECT_EQ(newWeights[i], trainer.getFeatureAppearCountsInLastTraining().at(i)/trainer.getCijofEjsInLastTraining().at(i));
          }
          //cout << "iter " << count+1 << " weight[" << i << "] = " << newWeights[i] << endl;
        }

        trainer.m_featureWeights = newWeights;
      }
    }

    //cout << "iterative update" << endl;
    {
      double expected_weights_per_iterations[3][6] = {
        {1.98008, 1.875, 0, 0, 0, 0},
        {3.15389, 2.10134, 0, 0, 0, 0},
        {4.4652, 2.26185, 0, 0, 0, 0}
      };
      int update_order[6] = {1,2,3,4,5,0};

      dataset.seekToBegin();
      trainer.initWeights();    
      int m_maxIterationCountForDataSet = 3;
      for (int count=0; count<m_maxIterationCountForDataSet; count++) {
        for (size_t j=0; j<trainer.m_featureWeights.size(); j++) {
          size_t i = update_order[j];
          vector<Board_Move_Dataset *> dataset_wrap;
          dataset_wrap.push_back(&dataset);

          trainer.m_featureWeights[i] = trainer.calculateNewOneDimensionOfWeights(dataset_wrap, i);
          trainer.m_featureWeights_inverse[i] = 1.0/trainer.m_featureWeights[i];
          EXPECT_LE(fabs(expected_weights_per_iterations[count][i]-trainer.m_featureWeights[i]), 0.001);

          if (expected_weights_per_iterations[count][i] != 0) {
            EXPECT_EQ(trainer.m_featureWeights[i], trainer.getFeatureAppearCountsInLastTraining()[i]/trainer.getCijofEjsInLastTraining()[i]);
          }
          //cout << "iter " << count+1 << " weight[" << i << "] = " << trainer.m_featureWeights[i] << endl;
        }
      }
    }
  }

  void test_setWeightsByAppearAndCijofEj() {
    trainer.initWeights();

    trainer.setWeightByAppearCountAndCijofEj(3, 2, 0.34);
    for (size_t i=0; i<trainer.m_featureWeights.size(); i++) {
      if (i==3) {
        EXPECT_EQ(2/0.34, trainer.m_featureWeights[i]);
      } else {
        EXPECT_EQ(1, trainer.m_featureWeights[i]);
      }
    }
  }

  class DummyFeatureExtractor_size7 : public ML::FeatureExtractor {
    void extractFromStateForAllMoves(const Go::Board *state, Common::Color turn, Common::SparseVector *featureTableToExtract, Common::SparseVector &forPassFeature, Common::PointSet &extractedLegalMoves, bool clearOldFeatures) const {
    }
    size_t getFeatureDimension() const {return 7;}
  };

  LightFeatureExtractor_ForTest extractor;
  MM_Learning trainer;

  vector<vector<shared_ptr<SparseVector> > > test_feature_set;

  MM_Learning_Test()
    : extractor()
    , trainer(extractor)
  {
  }
};

TEST_F(MM_Learning_Test, CalcProdOfFeaturesWeights) {
  test_CalcProdOfFeaturesWeights();
}

TEST_F(MM_Learning_Test, CalcProdOfTeammateWeights) {
  test_CalcProdOfTeammateWeights();
}

TEST_F(MM_Learning_Test, updateForVectorsOfOne) {
  test_updateForVectorsOfOne();
}

TEST_F(MM_Learning_Test, convertOneDataToFeatureVectors_in_Eye) {
  test_convertOneDataToFeatureVectors_in_Eye();
}

TEST_F(MM_Learning_Test, convertDataToLightFeatureVectors) {
  // check converting (board, move) to vector(features) is correct
  test_convertDataToLightFeatureVectors();
}

TEST_F(MM_Learning_Test, learnWithDataset) {
  // run Learning with (board, move) dataset
  test_learnWithDataset();
}

TEST_F(MM_Learning_Test, setWeightsByAppearAndCijofEj) {
  test_setWeightsByAppearAndCijofEj();
}
