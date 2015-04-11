#include "precomp.h"

#include "ML/MM_Learning.h"

#include "ML/Board_Move_Dataset.h"
#include "ML/FeatureArray.h"
#include "ML/FeatureExtractor.h"

#include <sys/time.h>

using namespace std;
using namespace Common;
using namespace Go;

namespace ML {

  typedef SingletonMemoryPool<FeatureValues, 1000> FeatureValuesAllocator;
  typedef SingletonMemoryPool<FeatureWeights, 1000> FeatureWeightsAllocator;

  MM_Learning::MM_Learning(FeatureExtractor &featureExtractor)
    : m_featureExtractor(featureExtractor)
      //, m_maxIterationCountForDataSet(0)
    , m_stepRate(0.0)
    , m_featureWeights(featureExtractor.getFeatureDimension())
    , m_featureWeights_inverse(featureExtractor.getFeatureDimension())
    , m_featureAppearCountsInLastTraining(featureExtractor.getFeatureDimension())
    , m_CijofEjsInLastTraining(featureExtractor.getFeatureDimension())
  {
    initWeights();
  }

  MM_Learning::MM_Learning(FeatureExtractor &featureExtractor, double stepRate)
    : m_featureExtractor(featureExtractor)
    , m_featureWeights(featureExtractor.getFeatureDimension())
    , m_featureWeights_inverse(featureExtractor.getFeatureDimension())
    , m_featureAppearCountsInLastTraining(featureExtractor.getFeatureDimension())
    , m_CijofEjsInLastTraining(featureExtractor.getFeatureDimension())
  {
    setParameters(stepRate);
    initWeights();
  }

  void MM_Learning::initWeights() {
    for (size_t i=0; i<m_featureWeights.size(); i++) {
      m_featureWeights[i] = m_featureWeights_inverse[i] = 1.0;
      m_featureAppearCountsInLastTraining[i] = 1;
      m_CijofEjsInLastTraining[i] = 1.0;
    }
  }

  void MM_Learning::setParameters(double stepRate) {
    //m_maxIterationCountForDataSet = maxIter;
    m_stepRate = stepRate;
  }

  void MM_Learning::learn(std::vector<Board_Move_Dataset *> &dataSet)
  {
    //learn_iterative(dataSet);
    learn_at_once(dataSet);
  }

  void MM_Learning::setWeightByAppearCountAndCijofEj(size_t dim, int appearCount, double CijofEj) {
    if (dim >= m_featureWeights.size()) return;
    m_featureWeights[dim] = appearCount == 0 ? 0 : appearCount / CijofEj;
    m_featureWeights_inverse[dim] = 1.0/m_featureWeights[dim];
  }

  void MM_Learning::setWeightsByAppearCountsAndCijofEjs(const FeatureValues &appearCounts, const FeatureWeights &CijofEjs) {
    for (size_t i=0; i<m_featureWeights.size(); i++) {
      setWeightByAppearCountAndCijofEj(i, appearCounts[i], CijofEjs[i]);
    }
  }

  //--------------------------- 
  // At once Updater methods
  //---------------------------
  void MM_Learning::learn_at_once(std::vector<Board_Move_Dataset *> &dataSet) {
    FeatureWeights newWeights(m_featureExtractor.getFeatureDimension());
    //for (int count=0; count<m_maxIterationCountForDataSet; count++) {
    //for (size_t i=0; i<m_featureWeights.size(); i++) {
    //  newWeights[i] = calculateNewOneDimensionOfWeights(dataSet, i);
    //}
    //m_featureWeights = newWeights;
    timeval begin, end;
    gettimeofday(&begin, NULL);
    cerr << "Start iteration " << endl;
    m_featureWeights = calculateNewWeightsAtOnce(dataSet);
    for (size_t i=0; i<m_featureWeights.size(); i++) m_featureWeights_inverse[i] = 1.0/m_featureWeights[i];
    gettimeofday(&end, NULL);
    cerr << "Finished. update time (sec): " << -(begin.tv_sec - end.tv_sec + (1e-6)*(begin.tv_usec - end.tv_usec)) << endl; 
    //}
  }

  FeatureWeights MM_Learning::calculateNewWeightsAtOnce(std::vector<Board_Move_Dataset *> &dataSet) {
    Point freePointBuffer[MAX_BOARD_SIZE+1];
    FeatureValues totalAppearCounts(m_featureWeights.size());
    FeatureWeights total_CijofEjs(m_featureWeights.size());
    FeatureWeights newWeights(m_featureWeights.size());
    totalAppearCounts.clear();
    total_CijofEjs.clear();
    newWeights.clear();

    m_featureAppearCountsInLastTraining.clear();
    m_CijofEjsInLastTraining.clear();

    FeatureValues appearCounts(m_featureWeights.size());
    FeatureWeights CijofEjs(m_featureWeights.size());

    int count = 1;
    for (size_t i=0; i<dataSet.size(); i++) {
      Board_Move_Dataset &set = *dataSet[i];
      set.seekToBegin();
      if (!set.empty()) {
        do {
          //cerr << "begin dataset " << count << endl;
          Board_Move_Data data(set.get());
          //CijofEjs.clear(0);
          //appearCounts.clear(0);

          updateForOneData(data, CijofEjs, appearCounts, freePointBuffer);        
          
          totalAppearCounts += appearCounts;
          // accumulate Cij/Ej
          total_CijofEjs += CijofEjs;
        } while (set.next());
        //cerr << "end dataset " << count << endl;
        count++;
      }
    }

    m_featureAppearCountsInLastTraining = totalAppearCounts;
    m_CijofEjsInLastTraining = total_CijofEjs;
    
    for (size_t i=0; i<m_featureWeights.size(); i++) {
      newWeights[i] = totalAppearCounts[i] == 0 ? 0 : totalAppearCounts[i] / total_CijofEjs[i];
    }

    return newWeights;
  }

  void MM_Learning::updateForOneData(const Board_Move_Data &data, FeatureWeights &CijofEjs, FeatureValues &featureCounts, Common::Point *freePointBuffer) {
    vector<SparseVector *> featureVectors;
    int index_of_action_in_data = -1;
    convertOneDataToFeatureVectors(m_featureExtractor, data, freePointBuffer, featureVectors, index_of_action_in_data);

    if (featureVectors.size() == 0) {
      featureCounts.clear();
      CijofEjs.clear();
      cerr << "No features in training data" << endl;
    } else {
      if (!(index_of_action_in_data >= 0 && index_of_action_in_data < (signed)featureVectors.size())) {
        cerr << "Move = " << data.move << " vector size = " << featureVectors.size() << endl;
      }
      assert(index_of_action_in_data >= 0 && index_of_action_in_data < (signed)featureVectors.size());
      updateForFeatureVectorsOfOneData(featureVectors, index_of_action_in_data, CijofEjs, featureCounts);
    }

    releaseSparseVectors(featureVectors);
  }

  // Perform calculations for one data (data is already converted to feature vectors)
  void MM_Learning::updateForFeatureVectorsOfOneData(vector<SparseVector *> &featureVectors, int index_of_action_in_data, FeatureWeights &CijofEjs, FeatureValues &featureCountAppearInTraining) {

    featureCountAppearInTraining.clear();
    CijofEjs.clear();
    double total_product_of_weights = 0.0;  // Ej
    assert(CijofEjs.size() == m_featureWeights.size());

    int size = (signed)featureVectors.size();

    for (int i=0; i<size; i++) {
      SparseVector *features(featureVectors.at(i));
      //cerr << features->toString() << endl;
      
      // 1.2 calculate product of weights (a part of Ej)
      double prod = calculateProductOfExistFeatureWeights(*features);
      
      // 2: accumulate prod for Ej
      total_product_of_weights += prod;
      //cout << "prod = " << prod << endl;

      //const std::vector<int> &indices = features->getList();
      //const std::vector<int> &indices = features->getList();
      for (size_t j=0; j<features->size(); j++) {
        // 1.3 calculate part of Cij and accumulate it
        int index = features->at(j);
        CijofEjs[index] +=
          calculateProductOfTeammateWeights(*features, index, prod);
        if (index_of_action_in_data == i) {
          featureCountAppearInTraining[index]++;
        }
      }
    }

    // calculate Cij/Ej
    //CijofEjs = total_teammate_products;
    CijofEjs *= (1.0/total_product_of_weights);
  }

  //-----------------------------------------
  // Iterative Update (exact update) methods
  //-----------------------------------------
  void MM_Learning::learn_iterative(std::vector<Board_Move_Dataset *> &dataSet) {
    //for (int count=0; count<m_maxIterationCountForDataSet; count++) {
    for (size_t i=0; i<m_featureWeights.size(); i++) {
      learn_one_dimension(dataSet, i);
      //cerr << "Dimension " << i << ": " << m_featureWeights[i] << endl;
    }
    //}
  }

  void MM_Learning::learn_one_dimension(std::vector<Board_Move_Dataset *> &dataSet, size_t updateDim) {
    timeval begin, end;
    gettimeofday(&begin, NULL);
    cerr << "Start updating dimension " << updateDim << " ... ";
    m_featureWeights[updateDim] = calculateNewOneDimensionOfWeights(dataSet, updateDim);
    m_featureWeights_inverse[updateDim] = 1.0/m_featureWeights[updateDim];
    gettimeofday(&end, NULL);
    cerr << "Finished. update time (sec): " << -(begin.tv_sec - end.tv_sec + (1e-6)*(begin.tv_usec - end.tv_usec)) << endl; 

  }


  // if updateDim == (size_t)(-1), update all dimensions
  double MM_Learning::calculateNewOneDimensionOfWeights(std::vector<Board_Move_Dataset *> &dataSet, size_t updateDim) {
    // Update formula which imaximize: ln(L)
    //  weights_i = feature(i)_appear_count / (sum(Cij/Ej) for all j)
    //  i: weight index, j: dataset index

    // 1. 合法手を enumerate
    //  1.1. それぞれの手を打った時のFeatureを取り出す
    //  1.2. Feature が1になっている要素についてweightの積を取る
    //  1.3. feature ごとに、weightの積/weight[featureIndex] を和として保存しておく => Cij
    // 2. 全合法手について1.2で計算したものを総和 = Ej
    // 3. data に記述されている move についてFeatureを取り出す
    // 4. Wvec += 3で取り出したfeature
    // j: データセットごとのイテレーションカウンタ
    // Wi: 全データセット中、要素iが出てきた回数 (3で取り出したfeature中で1になっていたもの)
    //  Wvec = (W1, W2, ..., Wn) <= 全データセットでの feature の合計に相当

    // 論文では batch update すべき！と書いてあるが、本では iterative にやれ、と書いてある... 依存関係あると batch だめ？

    Point freePointBuffer[MAX_BOARD_SIZE+1];
    int totalFeatureAppearCount = 0;
    double total_CijofEj = 0;

    for (size_t i=0; i<dataSet.size(); i++) {
      Board_Move_Dataset &set = *dataSet[i];
      set.seekToBegin();
      if (!set.empty()) {
        do {
          const Board_Move_Data &data = set.get();
          
          double CijofEj = 0.0;
          int appearCountInData = 0;
          
          updateForOneData(data, updateDim, CijofEj, appearCountInData, freePointBuffer);
          
          totalFeatureAppearCount += appearCountInData;
          // accumulate Cij/Ej
          total_CijofEj += CijofEj;
        } while (set.next());
        //cerr << "end dataset " << i << " of dimension " << updateDim << endl;
      }
    }

    m_featureAppearCountsInLastTraining[updateDim] = totalFeatureAppearCount;
    m_CijofEjsInLastTraining[updateDim] = total_CijofEj;

    // update Each elements (this should be batch update?)
    // weights[i] = appear_count[i] /  (total_CijofEj[i])
    //cout << "appear = " << totalFeatureAppearCount << " CijofEj = " << total_CijofEj << endl;
    assert(totalFeatureAppearCount == 0 || total_CijofEj != 0.0);
    //cerr << "Appear Count = " << totalFeatureAppearCount << " CijofEj = " << total_CijofEj << " Weight = " << (totalFeatureAppearCount)/(total_CijofEj) << endl;
    return totalFeatureAppearCount == 0 ? 0 : totalFeatureAppearCount / total_CijofEj;
  }

  // interface of convertOneDataToFeatureVectors and updateForFeatureVectorsOfOneData
  void MM_Learning::updateForOneData(const Board_Move_Data &data, size_t updateDim, double &CijofEj, int &featureCountAppearInTraining, Point *freePointBuffer) {

    vector<SparseVector *> featureVectors;
    int index_of_action_in_data = -1;
    bool isAppear = false;
    convertOneDataToFeatureVectors(m_featureExtractor, data, freePointBuffer, featureVectors, index_of_action_in_data, &updateDim, &isAppear);
    if (featureVectors.size() == 0) {
      featureCountAppearInTraining = 0;
      CijofEj = 0;
      cerr << "No features in training data" << endl;
      return;
    }

/*    int count = 0;
      for (int i=0; i<(signed)featureVectors.size(); i++) {
      if (featureVectors[i]->at(updateDim) != 0) {
      count++; break;
      }
      }*/
    if (!isAppear) {
      featureCountAppearInTraining = 0;
      CijofEj = 0;
    } else {

      // if updateDim of all featureVectors are 0, convertOneDataToFeatureVectors is meanless. skip it
      
      assert(index_of_action_in_data >= 0 && index_of_action_in_data < (signed)featureVectors.size());
      updateForFeatureVectorsOfOneData(featureVectors, index_of_action_in_data, updateDim, CijofEj, featureCountAppearInTraining);
    }

    releaseSparseVectors(featureVectors);
  }

  // Perform calculations for one data (data is already converted to feature vectors)
  void MM_Learning::updateForFeatureVectorsOfOneData(vector<SparseVector *> &featureVectors, int index_of_action_in_data, size_t updateDim, double &CijofEj, int &featureCountAppearInTraining) {

    featureCountAppearInTraining = 0;
    double total_product_of_weights = 0.0;  // Ej
    double total_teammate_products = 0.0;  // Cij (i = updateDim)

    for (int i=0; i<(signed)featureVectors.size(); i++) {
      SparseVector *features(featureVectors.at(i));
      
      // 1.2 calculate product of weights (a part of Ej)
      double prod = calculateProductOfExistFeatureWeights(*features);
      
      // 2: accumulate prod for Ej
      total_product_of_weights += prod;
      //cout << "prod = " << prod << endl;

      if (features->contains(updateDim)) {
        // 1.3 calculate part of Cij and accumulate it
        total_teammate_products += 
          calculateProductOfTeammateWeights(*features, updateDim, prod);
        //cout << "appear!" << endl;

        if (i == index_of_action_in_data) {
          // accumulate Wi
          // this feature vector indicates the move which is in training data

          //  Wvec = (W1, W2, ..., Wn) の計算 <= 全データセットでの feature の合計に相当
          featureCountAppearInTraining++;
        }
      }
    }

    // calculate Cij/Ej
    CijofEj = total_teammate_products / total_product_of_weights;
    //cout << "CijofEj = " << CijofEj << endl; 
  }

  //------------------------------
  // Common Tools for Update
  //------------------------------

  // Calculate a part of Ej
  double MM_Learning::calculateProductOfExistFeatureWeights(const SparseVector &features) {
    double prod = 1.0;
    //const std::vector<int> &l = features.getList();
    for (size_t fi = 0; fi<features.size(); fi++) {
      prod *= m_featureWeights[features[fi]];
    }
    return prod;
  }

  // Calculate a part of Cij (in these arguments, i is equal to index)
  double MM_Learning::calculateProductOfTeammateWeights(const SparseVector &features, size_t index, double allProductOfExistFeatureWeights) {
    assert(features.contains(index));
    if (m_featureWeights[index] > 0.001) {
      return allProductOfExistFeatureWeights*m_featureWeights_inverse[index];
      //return allProductOfExistFeatureWeights/m_featureWeights[index];
    } else {
      //const std::vector<int> &l = features.getList();
      double prod = 1;
      for (size_t i=0; i<features.size(); i++) {
        if (features[i]!=(signed)index) {
          prod *= m_featureWeights[features[i]];
        }
      }
      return prod;
    }
  }

  void MM_Learning::releaseSparseVectors(std::vector<SparseVector *> &featureVectors) {
    for (size_t i=0; i<featureVectors.size(); i++) {
      featureVectors[i]->clear();
      SparseVectorAllocator::getInstance().release(featureVectors[i]);
    }
  }
}
