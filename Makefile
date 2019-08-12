all: rangerocksdb

rangerocksdb: rangerocksdb rangerocksdb.o
	$(CXX) $a.cc -L/home/josue/Devel/MCS/rocksdb -llibrocksdb -I../rocksdb/include/ -std=c++11 lpthread -lrt -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

clean:
	rm -rf ./rangerocksdb
