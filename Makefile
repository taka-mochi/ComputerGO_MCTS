CC=g++
MPICC=mpic++
GO_MAIN_OUT = go_main
CFLAGS_RELEASE  += -Wall -std=c++0x -O3 -DNDEBUG -march=native
CFLAGS_RELEASE_PERF += -Wall -std=c++0x -O3 -DNDEBUG -pg -march=native
CFLAGS_DEBUG_O3 += -pg -Wall -g -std=c++0x -O3 -DDEBUG
CFLAGS_DEBUG_O1 += -pg -Wall -g -std=c++0x -O1 -DDEBUG
CFLAGS_DEBUG_O0 += -pg -Wall -g -std=c++0x -O0 -DDEBUG
CFLAGS_DEBUG_MPI += -pg -Wall -g -std=c++0x -O3 -DDEBUG -DUSE_MPI
CFLAGS_RELEASE_MPI += -Wall -std=c++0x -O3 -DNDEBUG -DUSE_MPI
LDFLAGS_RELEASE = -O3 -march=native -lrt
LDFLAGS_RELEASE_PERF = -pg -O3 -march=native -lrt
LDFLAGS_DEBUG_O3 = -pg -O3 -lrt
LDFLAGS_DEBUG_O1 = -pg -O1 -lrt
LDFLAGS_DEBUG_O0 = -pg -O0 -lrt
CFLAGS =
LDFLAGS =
PRECOMP = precomp.h.gch
OBJ = main.o ExecuteParameter.o common.o \
	Go/Board.o Go/Block.o Go/Board_MoveChangeEntry.o Go/Rules.o Go/BoardHash.o Go/GoBook.o \
	ML/StandardFeatureExtractor.o ML/PatternExtractor_3x3.o \
	AI/RandomPlayer.o AI/NormalPlayout.o AI/UCTNode.o AI/UCTNodeAllocator.o AI/Parameters.o AI/SoftmaxPolicyPlayout.o AI/HistoryHeuristics.o AI/HistoryHeuristicsPlayout.o \
	Gtp/GtpCore.o Gtp/gtp_mode_main.o utility/utility.o utility/random.o
MLOBJ = Go/Rules.o Go/Board.o Go/Block.o Go/Board_MoveChangeEntry.o common.o Go/BoardHash.o \
	ML/ml_main.o ML/MM_Learning.o ML/Board_Move_Dataset.o ML/StandardFeatureExtractor.o ML/LightFeatureExtractor_ForTest.o ML/PatternExtractor_3x3.o ML/Board_Move_Dataset_record.o ML/ApprenticeshipLearning.o ML/GoLearning.o \
	utility/utility.o Record/Record.o Record/SgfReader.o utility/random.o
TESTOBJ = ExecuteParameter.o common.o \
	tests/test_board.o tests/test_main.o tests/test_ai.o tests/test_uctnode.o tests/test_softmaxpolicy_playout.o tests/test_parameters.o tests/test_board_hash.o tests/test_book.o tests/test_historyheuristics.o \
	Go/Board.o Go/Block.o Go/Board_MoveChangeEntry.o Go/Rules.o Go/BoardHash.o Go/GoBook.o \
	ML/StandardFeatureExtractor.o ML/PatternExtractor_3x3.o \
	AI/RandomPlayer.o AI/NormalPlayout.o AI/UCTNode.o AI/UCTNodeAllocator.o AI/Parameters.o AI/SoftmaxPolicyPlayout.o AI/HistoryHeuristics.o AI/HistoryHeuristicsPlayout.o \
	Gtp/GtpCore.o utility/utility.o utility/random.o
MLTESTOBJ = common.o \
	tests/ml_test_main.o tests/ml_test_features.o tests/ml_test_mm_learn.o tests/ml_test_patterns.o tests/ml_test_standard_feature_extractor.o tests/ml_test_board_move_dataset_record.o tests/ml_test_apprenticeship_learning.o tests/ml_test_cost_function.o \
	Go/Board.o Go/Block.o Go/Board_MoveChangeEntry.o Go/Rules.o Go/BoardHash.o \
	ML/MM_Learning.o ML/Board_Move_Dataset.o ML/StandardFeatureExtractor.o ML/LightFeatureExtractor_ForTest.o ML/PatternExtractor_3x3.o ML/Board_Move_Dataset_record.o ML/ApprenticeshipLearning.o ML/GoLearning.o \
	utility/utility.o Record/Record.o Record/SgfReader.o utility/random.o
DECODEROBJ = Go/Rules.o Go/Board.o Go/Block.o Go/Board_MoveChangeEntry.o common.o Go/BoardHash.o \
	ML/pattern_decoder.o ML/MM_Learning.o ML/Board_Move_Dataset.o ML/StandardFeatureExtractor.o ML/LightFeatureExtractor_ForTest.o ML/PatternExtractor_3x3.o ML/Board_Move_Dataset_record.o ML/GoLearning.o \
	utility/utility.o Record/Record.o Record/SgfReader.o utility/random.o
COSTVALUECALCOBJ = Go/Rules.o Go/Board.o Go/Block.o Go/Board_MoveChangeEntry.o common.o Go/BoardHash.o \
	ML/const_value_calculator.o ML/MM_Learning.o ML/Board_Move_Dataset.o ML/StandardFeatureExtractor.o ML/LightFeatureExtractor_ForTest.o ML/PatternExtractor_3x3.o ML/Board_Move_Dataset_record.o ML/ApprenticeshipLearning.o ML/GoLearning.o \
	utility/utility.o Record/Record.o Record/SgfReader.o utility/random.o

AGREEMENTOBJ = agreement_rate_calculator.o ExecuteParameter.o common.o \
	Go/Board.o Go/Block.o Go/Board_MoveChangeEntry.o Go/Rules.o Go/BoardHash.o \
	ML/StandardFeatureExtractor.o ML/PatternExtractor_3x3.o ML/Board_Move_Dataset_record.o \
	AI/RandomPlayer.o AI/NormalPlayout.o AI/UCTNode.o AI/UCTNodeAllocator.o AI/Parameters.o AI/SoftmaxPolicyPlayout.o AI/HistoryHeuristics.o AI/HistoryHeuristicsPlayout.o \
	utility/utility.o Record/Record.o Record/SgfReader.o utility/random.o

INCL = -I./ -I../../root/include
TESTLIB = -L../gtest/lib/
TESTINCL = -I../gtest/include/ -I../../root/lib

all: debug_o1

.PHONY: debug_o0
debug_o0: CFLAGS = $(CFLAGS_DEBUG_O0)
debug_o0: LDFLAGS = $(LDFLAGS_DEBUG_O0)
debug_o0: GO_MAIN_OUT = go_main_debugo0
debug_o0: go_main

.PHONY: debug_o1
debug_o1: CFLAGS = $(CFLAGS_DEBUG_O1)
debug_o1: LDFLAGS = $(LDFLAGS_DEBUG_O1)
debug_o1: GO_MAIN_OUT = go_main_debugo1
debug_o1: go_main

.PHONY: debug_o3
debug_o3: CFLAGS = $(CFLAGS_DEBUG_O3)
debug_o3: LDFLAGS = $(LDFLAGS_DEBUG_O3)
debug_o3: GO_MAIN_OUT = go_main_debugo3
debug_o3: go_main

.PHONY: release
release: CFLAGS = $(CFLAGS_RELEASE)
release: LDFLAGS = $(LDFLAGS_RELEASE)
release: GO_MAIN_OUT = go_main_release
release: go_main

.PHONY: release_perf
release_perf: CFLAGS = $(CFLAGS_RELEASE_PERF)
release_perf: LDFLAGS = $(LDFLAGS_RELEASE_PERF)
release_perf: GO_MAIN_OUT = go_main_releaseperf
release_perf: go_main

#go_main: $(PRECOMP) $(OBJ)
#	$(CC)  -o $(GO_MAIN_OUT) $(OBJ) -lpthread $(LDFLAGS)

go_main: $(OBJ)
	$(CC)  -o $(GO_MAIN_OUT) $(OBJ) -lpthread $(LDFLAGS)

.PHONY: ml
ml: CFLAGS = $(CFLAGS_DEBUG_O1)
ml: LDFLAGS = $(LDFLAGS_DEBUG_O1)
ml: ml_main

.PHONY: ml_debug_o3
ml_debug_o3: CFLAGS = $(CFLAGS_DEBUG_O3)
ml_debug_o3: LDFLAGS = $(LDFLAGS_DEBUG_O3)
ml_debug_o3: ml_main

.PHONY: ml_release
ml_release: CFLAGS = $(CFLAGS_RELEASE)
ml_release: LDFLAGS = $(LDFLAGS_RELEASE)
ml_release: ml_main

.PHONY: ml_release_perf
ml_release_perf: CFLAGS = $(CFLAGS_RELEASE_PERF)
ml_release_perf: LDFLAGS = $(LDFLAGS_RELEASE_PERF)
ml_release_perf: ml_main

.PHONY: ml_debug_mpi
ml_debug_mpi: CFLAGS = $(CFLAGS_DEBUG_MPI)
ml_debug_mpi: LDFLAGS = $(LDFLAGS_DEBUG_O3)
ml_debug_mpi: CC = $(MPICC)
ml_debug_mpi: ml_main

.PHONY: ml_release_mpi
ml_release_mpi: CFLAGS = $(CFLAGS_RELEASE_MPI)
ml_release_mpi: LDFLAGS = $(LDFLAGS_RELEASE)
ml_release_mpi: CC = $(MPICC)
ml_release_mpi: ml_main

ml_main: $(PRECOMP) $(MLOBJ)
	$(CC) $(LDFLAGS) -o $@ $(MLOBJ) -lpthread

.PHONY: pattern_decoder
pattern_decoder: CFLAGS = $(CFLAGS_DEBUG_O1)
pattern_decoder: LDFLAGS = $(LDFLAGS_DEBUG_O1)
pattern_decoder: pattern_decoder_main

pattern_decoder_main: $(PRECOMP) $(DECODEROBJ)
	$(CC) $(LDFLAGS) -o $@ $(DECODEROBJ) -lpthread

.PHONY: cost_value_calculator
cost_value_calculator: CFLAGS = $(CFLAGS_DEBUG_O1)
cost_value_calculator: LDFLAGS = $(LDFLAGS_DEBUG_O1)
cost_value_calculator: cost_value_calculator_main

cost_value_calculator_main: $(PRECOMP) $(COSTVALUECALCOBJ)
	$(CC) $(LDFLAGS) -o $@ $(COSTVALUECALCOBJ) -lpthread

.PHONY: cost_value_calculator_release
cost_value_calculator_release: CFLAGS = $(CFLAGS_RELEASE)
cost_value_calculator_release: LDFLAGS = $(LDFLAGS_RELEASE)
cost_value_calculator_release: cost_value_calculator_main

.PHONY: test
test: CFLAGS = $(CFLAGS_DEBUG_O0)
test: LDFLAGS = $(LDFLAGS_DEBUG_O0)
test: test_main

.PHONY: test_o1
test_o1: CFLAGS = $(CFLAGS_DEBUG_O1)
test_o1: LDFLAGS = $(LDFLAGS_DEBUG_O1)
test_o1: test_main

.PHONY: test_fast
test_fast: CFLAGS = $(CFLAGS_DEBUG_O3)
test_fast: LDFLAGS = $(LDFLAGS_DEBUG_O3)
test_fast: test_main

test_main: $(PRECOMP) $(TESTOBJ)
	$(CC) -o test $(TESTOBJ) $(TESTLIB) -lgtest -lpthread $(LDFLAGS)
	#./test

.PHONY: test_ml
test_ml: CFLAGS = $(CFLAGS_DEBUG_O0)
test_ml: LDFLAGS = $(LDFLAGS_DEBUG_O0)
test_ml: test_ml_main

.PHONY: test_ml_o1
test_ml_o1: CFLAGS = $(CFLAGS_DEBUG_O1)
test_ml_o1: LDFLAGS = $(LDFLAGS_DEBUG_O1)
test_ml_o1: test_ml_main

.PHONY: test_ml_fast
test_ml_fast: CFLAGS = $(CFLAGS_DEBUG_O3)
test_ml_fast: LDFLAGS = $(LDFLAGS_DEBUG_O3)
test_ml_fast: test_ml_main

test_ml_main: $(PRECOMP) $(MLTESTOBJ)
	$(CC) -o test_ml $(MLTESTOBJ) $(TESTLIB) -lgtest -lpthread -lrt

.PHONY: agreement
agreement: CFLAGS = $(CFLAGS_DEBUG_O1)
agreement: LDFLAGS = $(LDFLAGS_DEBUG_O1)
agreement: agreement_main

.PHONY: agreement_release
agreement_release: CFLAGS = $(CFLAGS_RELEASE)
agreement_release: LDFLAGS = $(LDFLAGS_RELEASE)
agreement_release: agreement_main

agreement_main: $(PRECOMP) $(AGREEMENTOBJ)
	$(CC) -o agreement_calculator $(AGREEMENTOBJ) -lpthread -lrt

precomp.h.gch: precomp.h common.h Go/Rules.h Go/Board.h Go/Block.h utility/SparseVector.h
main.o: precomp.h Go/Board.h Go/Block.h common.h AI/PureMCAI.h AI/UCTPlayer.h Gtp/GtpCore.h main_functions.h
ExecuteParameter.o: precomp.h ExecuteParameter.h
common.o: common.h
Go/Board.o: precomp.h Go/Board.h Go/Block.h common.h utility/PointMap.h
Go/Board_MoveChangeEntry.o: precomp.h Go/Board.h
Go/Block.o: precomp.h Go/Board.h Go/Block.h common.h utility/PointMap.h utility/SingletonMemoryPool.h
Go/Rules.o: precomp.h Go/Rules.h
AI/PureMCAI.o: precomp.h AI/PureMCAI.h common.h
AI/RandomPlayer.o: precomp.h AI/RandomPlayer.h common.h
AI/NormalPlayout.o: precomp.h AI/NormalPlayout.h Go/Board.h
AI/SoftmaxPolicyPlayout.o: precomp.h AI/SoftmaxPolicyPlayout.h Go/Board.h
AI/UCTNode.o: precomp.h AI/UCTNode.h Go/Board.h AI/UCTNodeAllocator.h
AI/UCTNodeAllocator.o: precomp.h AI/UCTNodeAllocator.h
AI/UCTPlayer.h: precomp.h AI/UCTBoundCalculator.h AI/UCTNodeAllocator.h AI/HistoryHeuristics.h AI/Parameters.h ML/StandardFeatureExtractor.h utility/utility.h
AI/HistoryHeuristics.o: precomp.h AI/HistoryHeuristics.h
AI/HistoryHeuristicsPlayout.o: precomp.h AI/HistoryHeuristics.h AI/HistoryHeuristicsPlayout.h
AI/Parameters.o: precomp.h AI/Parameters.h
Gtp/GtpCore.o: precomp.h Gtp/GtpCore.h
Gtp/gtp_mode_main.o: generated_version.h precomp.h main_functions.h
ML/ml_main.o: precomp.h ML/MM_Learning.h ML/Board_Move_Dataset.h ML/FeatureArray.h ML/FeatureExtractor.h ML/LearningRateInverseOfIteration.h ML/LearningRateSqrtOfInverseOfIteration.h ML/LearningRateConstant.h
ML/MM_Learning.o: precomp.h Go/Board.h ML/MM_Learning.h ML/Board_Move_Dataset.h ML/FeatureArray.h ML/FeatureExtractor.h
ML/Board_Move_Dataset.o: precomp.h ML/Board_Move_Dataset.h
ML/LightFeatureExtractor_ForTest.o: precomp.h ML/FeatureArray.h ML/FeatureExtractor.h ML/LightFeatureExtractor_ForTest.h utility/SparseVector.h
ML/StandardFeatureExtractor.o: precomp.h ML/FeatureArray.h ML/FeatureExtractor.h ML/StandardFeatureExtractor.h utility/SparseVector.h
ML/pattern_extractor.o: precomp.h Go/Board.h ML/pattern_extractor.h
ML/Board_Move_Dataset_record.o: precomp.h ML/Board_Move_Dataset_record.h ML/Board_Move_Dataset.h Record/Record.h
ML/pattern_decoder.o: ML/PatternExtractor_3x3.h
ML/ApprenticeshipLearning.o: precomp.h ML/ApprenticeshipLearning.h ML/FeatureExtractor.h
ML/GoLearning.o: precomp.h ML/GoLearning.h ML/Board_Move_Dataset.h common.h utility/SparseVector.h
Record/record.o: Record/Record.h
Record/SgfReader.o: Record/Record.h Record/SgfReader.h
utility/utility.o: utility/utility.h
utility/random.o: utility/utility.h
tests/test_ai.o: precomp.h AI/PureMCAI.h AI/RandomPlayer.h AI/UCTPlayer.h
tests/test_board.o: precomp.h Go/Board.h Go/Block.h common.h
tests/test_softmaxpolicy_playout.o: precomp.h AI/SoftmaxPolicyPlayout.h ML/StandardFeatureExtractor.h
tests/test_historyheuristics.o: AI/HistoryHeuristicsPlayout.h AI/UCTPlayer.h AI/UCTBoundCalculator.h
tests/test_parameters.o: precomp.h AI/Parameters.o
tests/ml_test_main.o: precomp.h
tests/ml_test_features.o: precomp.h ML/FeatureArray.h
tests/ml_test_mm_learn.o: precomp.h ML/FeatureArray.h ML/MM_Learning.h ML/FeatureExtractor.h ML/LightFeatureExtractor_ForTest.h ML/StandardFeatureExtractor.h
tests/ml_test_patterns: precomp.h ML/PatternExtractor_3x3.h
tests/ml_test_standard_feature_extractor.o: precomp.h ML/FeatureArray.h ML/FeatureExtractor.h ML/StandardFeatureExtractor.h ML/PatternExtractor_3x3.h
tests/ml_test_board_move_dataset_record.o: precomp.h ML/Board_Move_Dataset_record.h ML/Board_Move_Dataset.h
tests/ml_test_apprenticeship_learning.o: precomp.h ML/FeatureArray.h ML/ApprenticeshipLearning.h ML/LearningRateCalculator.h ML/LearningRateExponential.h
tests/ml_test_cost_function.o: precomp.h ML/FeatureArray.h ML/ApprenticeshipLearning.h ML/LearningRateCalculator.h ML/LearningRateExponential.h

.PHONY: generated_version.h
generated_version.h:
	echo -n const char *current_version=\" > generated_version.h
	git log -1 | grep Date | sed -e "s/Date://g" | sed -e "s/^[ ]*//g" | sed -e "s/ /_/g" | xargs echo -n >> generated_version.h
	echo \"\; >> generated_version.h

.cpp.o:
	$(CC) -o $@ -c $< $(CFLAGS) $(TESTINCL) $(INCL)

#precomp.h.gch:
#	$(CC) precomp.h $(CFLAGS) $(TESTINCL) $(INCL)

clean:
	rm -f *.out *.o tests/*.out tests/*.o AI/*.o utility/*.o Go/*.o Gtp/*.o ML/*.o *.h.gch Record/*.o
