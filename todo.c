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

void add_task(sqlite3 *db, const char *name,const char *due_date, const char *description)
{
    char *err_msg = 0;
    int rc;

    char *sql_template = "INSERT INTO Tasks (Name, DueDate, Description) VALUES ('%q', '%q', '%q');";
    char *sql = sqlite3_mprintf(sql_template, name, due_date, description)

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Task added successfully\n");
    }

    sqlite3_free(sql);
}

int main()
{
    printf("Hello, world!\n");
    initialize_db();
    printf("Initialized database\n");
    return 0;
}
