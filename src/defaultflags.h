#ifndef DEFAULT_FLAGS
#define DEFAULT_FLAGS
#include <gflags/gflags.h>

DEFINE_string(database, "rocksdb", "Database");
DEFINE_int64(queries, 100, "Number of queries");
DEFINE_int64(domain, 1000000, "Key domain size");
DEFINE_string(leaf_size, "100000", "Leaf size");
DEFINE_int64(range_size, 100000, "Range of queries");
DEFINE_bool(random_range_size, false, "Use random range of queries");
DEFINE_int64(min_range_size, 100000, "Min random range of queries");
DEFINE_int64(max_range_size, 100000, "Max random range of queries");
DEFINE_int64(percentage_point_queries, 0, "\% of Point Queries");
DEFINE_string(strategy, "lazy", "Strategy");
DEFINE_string(distribution, "normal", "Random Distribution");
DEFINE_int64(iter, 0, "Define the iteration number");
DEFINE_int64(seed, 100, "Random Seed");
DEFINE_bool(write_disk, false, "Write output to disk");
DEFINE_bool(exec_database, true, "Execute new query plan in Database");
DEFINE_bool(exec_checksum, false, "Show Checksum");
DEFINE_bool(exec_postfiltering, true, "Execute Postfiltering");

#endif // DEFAULT_FLAGS
