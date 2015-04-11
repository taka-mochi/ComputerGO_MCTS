
#pragma once

#include <vector>
#include "FeatureExtractor.h"
#include "ML/PatternExtractor_3x3.h"

#ifdef DEBUG
#define STDFEATURE_NOINLINE __attribute__((noinline))
#else
#define STDFEATURE_NOINLINE
#endif


namespace ML {
  /**************************************/
  /***** Standard Feature Extractor *****/
  /**************************************/
  class StandardFeatureExtractor : public FeatureExtractor{
  public:
    enum StaticFeatureIndex {
      // Distance = |dx| + |dy| + max(|dx|,|dy|). This feature means whether the last move is within the pattern
      STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4,
      //STATIC_FEATURE_PREVIOUS_MOVE_IS_PASS,
      STATIC_FEATURE_CURRENT_MOVE_IS_PASS,

      STATIC_FEATURE_CAPTURE_STRING_TO_NEW_ATARI,
      STATIC_FEATURE_CAPTURE_RE_CAPTURE,
      //STATIC_FEATURE_CAPTURE_PREVENT_CONNECTION_TO_PREVIOUS_MOVE,
      STATIC_FEATURE_CAPTURE,  // TODO

      STATIC_FEATURE_SELF_ATARI,

      STATIC_FEATURE_ATARI_WHEN_THERE_IS_A_KOU,
      STATIC_FEATURE_ATARI_OTHER_SITUATIONS,

      STATIC_FEATURE_DIST_FROM_BEFORE_PREVIOUS_IS_LESS_THAN_4,

      STATIC_FEATURE_EXTENSION_NEW_ATARI,

      STATIC_FEATURE_SIZE,

      STATIC_FEATURE_INVALID = -111
    };

  private:
    typedef std::vector<PATTERN_3x3_HASH_VALUE> PatternHashVector;
    typedef std::unordered_map<PATTERN_3x3_HASH_VALUE, int> HashIndexTable;

  public:
    struct NeighborBlockSetCache {
      bool needToRefresh;
      Go::Board::NeighborBlockSet blockSet;
      NeighborBlockSetCache()
      : needToRefresh(true)
      , blockSet()
      {
      }
    };

  private:

    PatternHashVector m_indexToHashValue;
    HashIndexTable m_hashValueToIndex;
    std::vector<PatternExtractor_3x3 *> m_patternExtractorsForEachSize;

    mutable NeighborBlockSetCache m_neighborBlockSetCacheMy[MAX_BOARD_SIZE], m_neighborBlockSetCacheEnemy[MAX_BOARD_SIZE];

    static void initNeighborBlockSetCacheTable(const Go::Board *board, NeighborBlockSetCache *table) STDFEATURE_NOINLINE;
    static Go::Board::NeighborBlockSet &getBlockSet(const Go::Board *board, Common::Point p, Common::Color c, NeighborBlockSetCache *cacheTable);

  private:
    static void _internal_enumerateMovesAndAddFeatureForDistanceFeature(StaticFeatureIndex index, Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *move, Common::SparseVector *featureTable, Common::Color player, bool doClearAddToSet) STDFEATURE_NOINLINE;

    static void _internal_updateStaticFeaturesForPreviousMoves(const Go::Board *state, const Go::Board::MoveChangeEntry *prevMove, Common::Color player, Common::SparseVector *featureTable, Common::PointSet &addTo, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy);

  public:
    explicit StandardFeatureExtractor();
    StandardFeatureExtractor(const StandardFeatureExtractor &rhs);
    ~StandardFeatureExtractor();

    //void extractFromStateAndAction(FeatureValues &result, const Go::Board *state, Common::Point action, Common::Color turn) const;
    //void extractFromStateAndAction(Common::SparseVector &sparse_result, const Go::Board *state, Common::Point action, Common::Color turn) const;

    void extractFromStateForAllMoves(const Go::Board *state, Common::Color turn, Common::SparseVector *featureTableToExtract, Common::SparseVector &forPassFeature, Common::PointSet &extractedLegalMoves, bool clearOldFeatures = true) const;
    void updateStaticFeaturesForAllMovesWithoutClearOldFeatures(const Go::Board *state, Common::Color player, const Common::PointSet &allLegalMoves, Common::SparseVector *featureTable, Common::PointSet &addMoves) const;
    
    size_t getFeatureDimension() const;

    void updatePatternFeature(Common::SparseVector &old_vector, const Go::Board *state, Common::Point action, Common::Color turn) const;
    //void updateStaticFeature(Common::SparseVector &old_vector, const Go::Board *state, Common::Point action, Common::Color turn) const;
    
    // check method which decides the given (state, action) includes that features
    static bool isRecapture(const Go::Board *state, Common::Point action, Common::Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy);
    static bool isSelfAtari(const Go::Board *state, Common::Point action, Common::Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) STDFEATURE_NOINLINE;
    static bool isCapture(const Go::Board *state, Common::Point action, Common::Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) STDFEATURE_NOINLINE;
    static StaticFeatureIndex checkAtariFeatures(const Go::Board *state, Common::Point action, Common::Color turn, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy) STDFEATURE_NOINLINE;

    // enumerate moves for static features
    static void enumerateMovesOfPatternChanged(const Go::Board *state, const Go::Board::MoveChangeEntry *checkHistory, Common::Color turn, Common::PointSet &moves, bool clearSet = false);
    static void enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(const Go::Board *state, Common::Color turn, Common::PointSet &moves, Common::PointSet &extensionMoves, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy);
    static void enumerateMovesAndAddFeatureForDistanceFromLastMoveIsLessThan4(Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *lastMove, Common::SparseVector *featureTable, Common::Color player, bool doClearAddToSet = false);
    static void enumerateMovesAndAddFeatureForDistanceFromSecondLastMoveIsLessThan4(Common::PointSet &addTo, const Go::Board *board, const Go::Board::MoveChangeEntry *secondLast, Common::SparseVector *featureTable, Common::Color player, bool doClearAddToSet = false);
    static void enumerateMovesAndAddFeaturesForFeatures_Capture_SelfAtari_Atari(Common::PointSet &addTo, const Go::Board *board, const Common::PointSet &allLegalMoves, Common::SparseVector *featureTable, Common::Color player, NeighborBlockSetCache *blockSetCacheTableMy, NeighborBlockSetCache *blockSetCacheTableEnemy, bool doClearAddToSet = false) STDFEATURE_NOINLINE;

    static void clearStaticFeatures(Common::SparseVector &vec);
    static void clearPatternFeatures(Common::SparseVector &vec);

    //void extractStaticFeatures(FeatureValues &result, const Go::Board *state, Common::Point action, Common::Color turn) const;
    //void extractPatternFeatures(FeatureValues &result, const PatternExtractor_3x3 &pattern_extractor, const Go::Board *state, Common::Point action, Common::Color turn) const;
    void extractStaticFeaturesToSparseVector(Common::SparseVector &result, const Go::Board *state, Common::Point action, Common::Color turn) const;
    void extractPatternFeaturesToSparseVector(Common::SparseVector &result, const PatternExtractor_3x3 &pattern_extractor, const Go::Board *state, Common::Point action, Common::Color turn) const;


    ////////////////////////////
    // for pattern extraction //
    ////////////////////////////
    int registerPatternAsFeature(PATTERN_3x3_HASH_VALUE pattern);
    int getFeatureIndexOfPattern(PATTERN_3x3_HASH_VALUE pattern) const;
    PATTERN_3x3_HASH_VALUE getPatternOfFeatureIndex(int index) const;

    const PatternExtractor_3x3 &getPatternExtractor_size9() const {
      return *m_patternExtractorsForEachSize[9];
    };
    const PatternExtractor_3x3 &getPatternExtractor_size19() const {
      return *m_patternExtractorsForEachSize[19];
    };
    const PatternExtractor_3x3 &getPatternExtractor(size_t size) const {
      return *m_patternExtractorsForEachSize[size];
    };

    static bool readPatternsAndWeightsFromFile(const std::string &weight_file, std::vector<PATTERN_3x3_HASH_VALUE> &values, std::vector<std::shared_ptr<FeatureWeights> > &weights);
  };
}
