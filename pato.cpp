#include "duckdb.hpp"

using namespace duckdb;
using namespace std;

int main() {
	DuckDB db("/dev/shm/ducks");
	Connection con(db);

	// con.Query("CREATE TABLE simple(key INTEGER, value INTEGER)");
	// con.Query("INSERT INTO simple VALUES (3, 4)");
	auto result = con.Query("SELECT COUNT(*) FROM simple");
	result->Print();

    return 0;
}
