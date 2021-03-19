CXX := g++
RM := rm -f
CXXFLAGS := -std=c++11 -O3 -g
LDFLAGS := -I/home/josue/Devel/MCS/dbs/duckdb/src/include -L/home/josue/Devel/MCS/dbs/duckdb/build/release/src -lduckdb -lrocksdb -lpthread -lrt -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd -ldl

COOPLANNING = cooplanning
CSVHEADER = csvheader
WRITES = writes

all: $(COOPLANNING) $(CSVHEADER) $(WRITES)

$(COOPLANNING): $(COOPLANNING).cpp
	$(CXX) $(COOPLANNING).cpp $(CXXFLAGS) $(LDFLAGS) -o $(COOPLANNING).out

$(CSVHEADER): $(CSVHEADER).cpp
	$(CXX) $(CSVHEADER).cpp $(CXXFLAGS) $(LDFLAGS) -o $(CSVHEADER).out

$(WRITES): $(WRITES).cpp
	$(CXX) $(WRITES).cpp $(CXXFLAGS) $(LDFLAGS) -o $(WRITES).out

clean:
	$(RM) count *.out *~
