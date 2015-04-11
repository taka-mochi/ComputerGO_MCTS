
#pragma once

#include "common.h"
#include "ML/FeatureArray.h"
#include "ML/StandardFeatureExtractor.h"
#include "utility/PointMap.h"
#include "utility/utility.h"
#include <vector>

namespace Go {
  class Board;
}

class Test_SoftmaxPolicyPlayout;

namespace AI {
  class SoftmaxPolicyPlayout {

    friend class ::Test_SoftmaxPolicyPlayout;

    std::vector<Common::Point> m_sequence;
    //ML::FeatureWeights m_expFeatureWeights;
    //ML::FeatureWeights m_invertedExpFeatureWeights;
    ML::FeatureWeights m_featureWeights;
    ML::StandardFeatureExtractor m_featureExtractor;
    //Common::StdlibRandom m_rnd;
    Common::MTRandom &m_rnd;

    //Common::Point m_candidatesBuffer[MAX_BOARD_SIZE];
    double m_probTableBlack[MAX_BOARD_SIZE], m_probTableWhite[MAX_BOARD_SIZE];
    Common::SparseVector m_featureTableBlack[MAX_BOARD_SIZE], m_featureTableWhite[MAX_BOARD_SIZE];

    Common::PointSet m_pointSetBuffer;

    Common::PointSet m_toResetStaticFeaturesMovesBlack, m_toResetStaticFeaturesMovesWhite;

  private:
    // internal methods
    //void initProbabilities(const Go::Board *initial_board);
    void initProbabilities(const Go::Board *init_board, Common::Color myTurn, double *table, Common::SparseVector *featureTable);
    void updateProbabilitiesBeforeAction(const Go::Board *board, Common::Color player);
    //void addUpdateCandidateMovesAndAddFeaturesForPreviousMoveFeatures(Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *move, Common::SparseVector *featureTable, Common::Color player);
    //void addUpdateCandidateMovesAndAddFeaturesForSecondLastMoveFeatures(Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *secondLast, Common::SparseVector *featureTable, Common::Color player);
    
    //void addUpdateCandidateMoves(Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *move, Common::Color player, bool checkCapture = true, Common::PointSet *daburiChecker = NULL);
    Common::Point selectOneMoveAccordingToProbabilityTable(Go::Board *board, double *table);
    //void addIllegalCandidateMoves(Common::PointSet &addTo, const Common::PointSet &updateTargets, const Go::Board *board, const Go::Board::MoveChangeEntry *move, Common::Color player);

    void enumerateMovesOfPatternChanged(Common::PointSet &addTo, const Go::Board *board, Common::Color player, const Go::Board::MoveChangeEntry *lastMove, const Go::Board::MoveChangeEntry *secondLastMove, bool doClear = false);
    void enumerateToBeLegalAndIllegalMoves(const Go::Board *board, double *probTable, Common::Color player, Common::PointSet &toBeLegal, Common::PointSet &toBeIllegal, Common::PointSet &allLegalMoves, bool clearResultSet = false);

  public:
    SoftmaxPolicyPlayout(const Go::Board *initial_board, const std::vector<ML::PATTERN_3x3_HASH_VALUE> &pattern_list, const ML::FeatureWeights &weights, Common::MTRandom &rnd);
    explicit SoftmaxPolicyPlayout(const std::string &learnedFeatureWeightFile, Common::MTRandom &rnd);
    virtual ~SoftmaxPolicyPlayout();
    std::string getName() const {
      return "SoftmaxWithStdFeature";
    }
    inline const std::vector<Common::Point> &getLastSequence() const {
      return m_sequence;
    }
    ML::StandardFeatureExtractor *getFeatureExtractor() {return &m_featureExtractor;}
    const ML::FeatureWeights *getFeatureWeights() const {return &m_featureWeights;}
    //const ML::FeatureWeights *getExpFeatureWeights() const {return &m_expFeatureWeights;}
    void setFeatureWeights(const ML::FeatureWeights &weights);
    void addFeatureWeights(const ML::FeatureWeights &weights);
    void setExpFeatureWeights(const ML::FeatureWeights &expWeights);
    double operator()(Common::Color turn_color, Go::Board *board, int depth, bool doUndo = true);

    void printProbabilityTableToErr(const Go::Board *board) const;
  };
}
