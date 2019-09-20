#include "rocksdb/db.h"


class NumericComparator : public rocksdb::Comparator {
public:
    // Three-way comparison function:
    //   if a < b: negative result
    //   if a > b: positive result
    //   else: zero result
    int Compare(const rocksdb::Slice& a, const rocksdb::Slice& b) const {
        int a1, b1;
        ParseKey(a, &a1);
        ParseKey(b, &b1);
        if (a1 < b1) return -1;
        if (a1 > b1) return +1;

        return 0;
    }

    void ParseKey(const rocksdb::Slice& a, int* a1) const {
    	std::string parts = a.ToString();
    	*a1 = atoi(parts.c_str());
    }

    const char* Name() const { return "NumericComparator"; }
    void FindShortestSeparator(std::string*, const rocksdb::Slice&) const { }
    void FindShortSuccessor(std::string*) const { }
};
