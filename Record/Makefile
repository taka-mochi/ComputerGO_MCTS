CC=g++
CFLAGS_RELEASE  += -Wall -std=c++0x -O3 -DNDEBUG
CFLAGS_RELEASE_PERF += -Wall -std=c++0x -O3 -DNDEBUG -pg
CFLAGS_DEBUG_O3 += -pg -Wall -g -std=c++0x -O3 -DDEBUG
CFLAGS_DEBUG_O1 += -pg -Wall -g -std=c++0x -O1 -DDEBUG
CFLAGS_DEBUG_O0 += -pg -Wall -g -std=c++0x -O0 -DDEBUG
LDFLAGS_RELEASE = -O3
LDFLAGS_RELEASE_PERF = -pg -O3
LDFLAGS_DEBUG_O3 = -pg -O3
LDFLAGS_DEBUG_O1 = -pg -O1
LDFLAGS_DEBUG_O0 = -pg -O0
CFLAGS =
LDFLAGS =
PRECOMP = ../precomp.h.gch
PATTERN_OBJ = pattern_extractor_from_sgf.o Record.o SgfReader.o ../common.o \
	../Go/Board.o ../Go/Block.o ../Go/Board_MoveChangeEntry.o ../Go/Rules.o \
	../ML/PatternExtractor_3x3.o ../utility/utility.o
TESTOBJ = test_main.o test_record.o test_sgf_reader.o Record.o SgfReader.o ../common.o \
	../Go/Board.o ../Go/Block.o ../Go/Board_MoveChangeEntry.o ../Go/Rules.o \
	../ML/PatternExtractor_3x3.o ../utility/utility.o
INCL = -I../
TESTLIB = -L../../gtest/lib/
TESTINCL = -I../../gtest/include/

.PHONY: debug_o3
debug_o3: CFLAGS = $(CFLAGS_DEBUG_O3)
debug_o3: LDFLAGS = $(LDFLAGS_DEBUG_O3)
debug_o3: all

all: pat_ext

.PHONY: debug_o1
debug_o1: CFLAGS = $(CFLAGS_DEBUG_O1)
debug_o1: LDFLAGS = $(LDFLAGS_DEBUG_O1)
debug_o1: all

.PHONY: debug_o0
debug_o0: CFLAGS = $(CFLAGS_DEBUG_O0)
debug_o0: LDFLAGS = $(LDFLAGS_DEBUG_O0)
debug_o0: all

.PHONY: release
release: CFLAGS = $(CFLAGS_RELEASE)
release: LDFLAGS = $(LDFLAGS_RELEASE)
release: all

.PHONY: release_perf
release_perf: CFLAGS = $(CFLAGS_RELEASE_PERF)
release_perf: LDFLAGS = $(LDFLAGS_RELEASE_PERF)
release_perf: all

pat_ext: $(PRECOMP) $(PATTERN_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(PATTERN_OBJ) -lpthread

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
	$(CC) -o test $(TESTOBJ) $(TESTLIB) -lgtest -lpthread
	#./test

../precomp.h.gch: ../precomp.h ../common.h ../Go/Rules.h ../Go/Board.h ../Go/Block.h
../common.o: ../common.h
pattern_extractor_from_sgf.o: Record.h SgfReader.h ../precomp.h.gch
Record.o: Record.h
SgfReader.o: SgfReader.h Record.h
../Go/board.o: ../precomp.h ../Go/board.h ../Go/blocks.h ../common.h ../utility/PointMap.h
../Go/board_movechangeentry.o: ../precomp.h ../Go/Board.h
../Go/blocks.o: ../precomp.h ../Go/Board.h ../Go/Block.h ../common.h ../utility/PointMap.h
../Go/rules.o: ../precomp.h ../Go/Rules.h
../ML/PatternExtractor_3x3.o: ../precomp.h ../Go/Board.o ../common.h ../ML/PatternExtractor_3x3.h
../utility/utility.o: ../utility/utility.h
test_main.o: ../precomp.h
test_record.o: ../precomp.h Record.h
test_sgf_reader.o: ../precomp.h Record.h SgfReader.h

.cpp.o:
	$(CC) -o $@ -c $< $(CFLAGS) $(TESTINCL) $(INCL)

../precomp.h.gch:
	$(CC) ../precomp.h $(CFLAGS) $(TESTINCL) $(INCL)

clean:
	rm -f *.out *.o ../*.out ../*.o ../*.h.gch
