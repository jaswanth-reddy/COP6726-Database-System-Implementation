CC = g++ -O2 -Wno-deprecated 

tag = -i

ifdef linux
tag = -n
endif

test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o  Pipe.o BigQ.o ViewStateHandler.o MetaDataHandler.o SortedDBFile.o Function.o HeapDBFile.o GenericDBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test.o
	$(CC) -o test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o  Pipe.o BigQ.o ViewStateHandler.o MetaDataHandler.o Function.o SortedDBFile.o HeapDBFile.o GenericDBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test.o -lfl -lpthread

a4-1.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o  Pipe.o BigQ.o ViewStateHandler.o MetaDataHandler.o SortedDBFile.o Function.o HeapDBFile.o GenericDBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test.o
	$(CC) -o a4-1.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o  Pipe.o BigQ.o ViewStateHandler.o MetaDataHandler.o Function.o SortedDBFile.o HeapDBFile.o GenericDBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test.o -lfl -lpthread

a3-test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o ViewStateHandler.o DBFile.o Pipe.o BigQ.o MetaDataHandler.o Function.o SortedDBFile.o HeapDBFile.o GenericDBFile.o RelOp.o Function.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o a3-test.o
	$(CC) -o a3-test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o ViewStateHandler.o DBFile.o Pipe.o BigQ.o Function.o MetaDataHandler.o SortedDBFile.o HeapDBFile.o GenericDBFile.o RelOp.o Function.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o a3-test.o -lfl -lpthread

a2-2test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o ViewStateHandler.o BigQ.o MetaDataHandler.o Function.o SortedDBFile.o HeapDBFile.o GenericDBFile.o DBFile.o Pipe.o y.tab.o lex.yy.o a2-2test.o
	$(CC) -o a2-2test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o ViewStateHandler.o BigQ.o Function.o MetaDataHandler.o SortedDBFile.o HeapDBFile.o GenericDBFile.o DBFile.o Pipe.o y.tab.o lex.yy.o a2-2test.o -lfl -lpthread

a2-test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o ViewStateHandler.o BigQ.o DBFile.o Pipe.o Function.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o a2-test.o
	$(CC) -o a2-test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o ViewStateHandler.o BigQ.o Function.o DBFile.o Pipe.o y.tab.o lex.yy.o a2-test.o -lfl -lpthread

a1-test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o  Pipe.o BigQ.o ViewStateHandler.o Function.o MetaDataHandler.o SortedDBFile.o HeapDBFile.o GenericDBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o a1-test.o
	$(CC) -o a1-test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o  Pipe.o BigQ.o Function.o ViewStateHandler.o MetaDataHandler.o SortedDBFile.o HeapDBFile.o GenericDBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o a1-test.o -lfl -lpthread

test.o: test.cc
	$(CC) -g -c test.cc

a3-test.o: a3-test.cc
	$(CC) -g -c a3-test.cc

a2-2test.o: a2-2test.cc
	$(CC) -g -c a2-2test.cc

a2-test.o: a2-test.cc
	$(CC) -g -c a2-test.cc

a1-test.o: a1-test.cc
	$(CC) -g -c a1-test.cc

Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc

Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
File.o: File.cc
	$(CC) -g -c File.cc

ViewStateHandler.o: ViewStateHandler.cc
	$(CC) -g -c ViewStateHandler.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc

Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

MetaDataHandler.o: MetaDataHandler.cc
	$(CC) -g -c MetaDataHandler.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

SortedDBFile.o: SortedDBFile.cc
	$(CC) -g -c SortedDBFile.cc

HeapDBFile.o: HeapDBFile.cc
	$(CC) -g -c HeapDBFile.cc

GenericDBFile.o: GenericDBFile.cc
	$(CC) -g -c GenericDBFile.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" y.tab.c
	g++ -c y.tab.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

yyfunc.tab.o: ParserFunc.y
	yacc -p "yyfunc" -b "yyfunc" -d ParserFunc.y
	#sed $(tag) yyfunc.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/"
	g++ -c yyfunc.tab.c

lex.yyfunc.o: LexerFunc.l
	lex -P yyfunc LexerFunc.l
	gcc  -c lex.yyfunc.c

# A sample Makefile for building both Google Mock and Google Test and
# using them in user tests.  This file is self-contained, so you don't
# need to use the Makefile in Google Test's source tree.  Please tweak
# it to suit your environment and project.  You may want to move it to
# your project's root directory.
#
# SYNOPSIS:
#
#   make [all]  - makes everything.
#   make TARGET - makes the given target.
#   make clean  - removes all files generated by make.
# Please tweak the following variable definitions as needed by your
# project, except GMOCK_HEADERS and GTEST_HEADERS, which you can use
# in your own targets but shouldn't modify.
# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file, or if you want to use
# a copy of Google Test at a different location.
GTEST_DIR = googletest/googletest
# Points to the root of Google Mock, relative to where this file is.
# Remember to tweak this if you move this file.
GMOCK_DIR = googletest/googlemock
# Where to find user code.
USER_DIR = test
# Flags passed to the preprocessor.
# Set Google Test and Google Mock's header directories as system
# directories, such that the compiler doesn't generate warnings in
# these headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include
# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread
# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = gtests
# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
# All Google Mock headers. Note that all Google Test headers are
# included here too, as they are #included by Google Mock headers.
# Usually you shouldn't change this definition.
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)
# House-keeping build targets.
all : $(TESTS)

# Builds gmock.a and gmock_main.a.  These libraries contain both
# Google Mock and Google Test.  A test should link with either gmock.a
# or gmock_main.a, depending on whether it defines its own main()
# function.  It's fine if your test only uses features from Google
# Test (and not Google Mock).
# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SRCS_ = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)
# For simplicity and to avoid depending on implementation details of
# Google Mock and Google Test, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Mock and
# Google Test compile fast and for ordinary users their source rarely
# changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GTEST_DIR)/src/gtest-all.cc
gmock-all.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock-all.cc
gmock_main.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock_main.cc
gmock.a : gmock-all.o gtest-all.o
	$(AR) $(ARFLAGS) $@ $^
gmock_main.a : gmock-all.o gtest-all.o gmock_main.o
	$(AR) $(ARFLAGS) $@ $^

StatisticsTest.o: $(USER_DIR)/StatisticsTest.cc $(GMOCK_HEADERS)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -g -c $(USER_DIR)/StatisticsTest.cc

gtests: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o  Pipe.o BigQ.o ViewStateHandler.o MetaDataHandler.o SortedDBFile.o HeapDBFile.o GenericDBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o StatisticsTest.o gmock_main.a
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.bin
	rm -f *.temp
	rm -f *.txt
	rm -f *.out
	rm -f y.tab.*
	rm -f yyfunc.tab.*
	rm -f lex.yy.*
	rm -f lex.yyfunc*
	rm -f gtests
