#include <stdio.h>
#include <sqlite3.h>

int main(int argc, char * argv[]) {
    sqlite3 * db;

    int rc = sqlite3_open16(":memory:", &db);

    char *zErrMsg = 0;
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    return 0;
}
