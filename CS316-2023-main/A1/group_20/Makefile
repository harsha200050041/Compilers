CXX = g++-8
EXE = iplC
CXXDEBUG = -g -Wall
CXXSTD = -std=c++11

.PHONY: all
all: parser lexer driver.cpp
	$(CXX) $(CXXDEBUG) $(CXXSTD) -o $(EXE) driver.cpp parser.o scanner.o 

.PHONY: parser
parser: parser.yy
	bison -d -v $<
	$(CXX) $(CXXDEBUG) $(CXXSTD) -c parser.tab.cc -o parser.o 

.PHONY: lexer
lexer: scanner.l scanner.hh
	flex++ --outfile=scanner.yy.cc $<
	$(CXX) $(CXXDEBUG) $(CXXSTD) -c scanner.yy.cc -o scanner.o

.PHONY: clean
clean:
	rm iplC location.hh position.hh stack.hh *.o *.output *.tab.cc *.tab.hh *.yy.cc