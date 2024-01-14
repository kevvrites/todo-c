#include <sqlite3.h>
#include <stdio.h>

void initialize_db()
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc;

    rc = sqlite3_open("todo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS Tasks("
                      "Id INTEGER PRIMARY KEY, "
                      "Name TEXT NOT NULL, "
                      "DueDate DATE, "
                      "Description TEXT);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    sqlite3_close(db);
}

int main()
{
    printf("Hello, world!\n");
    initialize_db();
    printf("Initialized database\n");
    return 0;
}
