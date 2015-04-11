#include "precomp.h"

#include "GoLearning.h"

using namespace std;
using namespace Common;
using namespace ML;
using namespace Go;


namespace ML {

  void GoLearning::convertOneDataToFeatureVectors(const FeatureExtractor &featureExtractor, const Board_Move_Data &data, Common::Point *freePointBuffer, std::vector<Common::SparseVector *> &result_buffer, int &index_of_action_in_data, size_t *checkAppearDim, bool *isAppear) {
    const Board *board = data.state.get();
    Point move = data.move;
    Color turn = data.turn;
    
    if (checkAppearDim) *isAppear = false;
    
    int freePointCount = 0;
    board->enumerateFreeMoves(freePointBuffer, freePointCount, turn);
    freePointBuffer[freePointCount++] = PASS; // add pass
    
    result_buffer.reserve(freePointCount);
    index_of_action_in_data = -1;

    bool isPutEyeAllowed = Go::Rules::isPutEyeAllowed();
    Go::Rules::setPutEyeAllowed(true);

    PointSet legalMoves;
    SparseVector featureTable[MAX_BOARD_SIZE];
    SparseVector *passFeatures(SparseVectorAllocator::getInstance().allocate());
    featureExtractor.extractFromStateForAllMoves(board, turn, featureTable, *passFeatures, legalMoves);

    //int featureAppearCounts = 0;
    for (size_t i=0; i<legalMoves.size(); i++) {
      Point p = legalMoves[i];

      //if (p-1 != PASS) {
        assert (board->checkLegalHand(legalMoves[i], turn, Board::flipColor(turn)) == Board::PUT_LEGAL);
        //}

      //if (p == PASS) p = 0;
      
      // OK. Legal move
      // 1.1 extract features
      SparseVector *features(SparseVectorAllocator::getInstance().allocate());
      result_buffer.push_back(features);

      // copy to the new SparseVector *
      for (size_t j=0; j<featureTable[p].size(); j++) {
        features->add(featureTable[p][j]);
        if (checkAppearDim != NULL && featureTable[p][j] == *checkAppearDim) {
          *isAppear = true;
        }
      }

      if (move == legalMoves[i]) {
        index_of_action_in_data = result_buffer.size()-1;
      }

    }
    Go::Rules::setPutEyeAllowed(isPutEyeAllowed);
    result_buffer.push_back(passFeatures);
    if (move == PASS) {
      index_of_action_in_data = result_buffer.size()-1;
    }
  }

}
