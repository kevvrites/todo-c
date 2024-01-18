#include <sqlite3.h>
#include <stdio.h>

typedef struct {
    const char *name;
    const char *category;
    const char *start_date;
    const char *due_date;
    const char *completion_date;
    const char *status;
    const char *priority;
    const char *description;
} Task;

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
                      "Category TEXT, "
                      "StartDate DATE, "
                      "DueDate DATE, "
                      "CompletionDate DATE, "
                      "Status TEXT, "
                      "Priority TEXT, "
                      "Description TEXT);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_close(db);
        return;
    }

    sqlite3_close(db);
    return;
}

void add_task(sqlite3 *db, const char *name, const char *category, const char *start_date, const char *due_date, const char *completion_date, const char *status, const char *priority, const char *description)
{
    sqlite3_stmt *stmt;
    int rc;
    const char *sql;

    sql = "INSERT INTO Tasks (Name, Category, StartDate, DueDate, CompletionDate, Status, Priority, Description) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    const char *task_data[] = {name, category, start_date, due_date, completion_date, status, priority, description};
    int num_params = sizeof(task_data) / sizeof(task_data[0]);

    for (int i = 0; i < num_params; i++) {
        sqlite3_bind_text(stmt, i + 1, task_data[i] ? task_data[i] : NULL, -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Task added successfully\n");
    }

}

static int list_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; i++) {
        printf("%s: %s\n", azColName[i], argv[i] ? argv[i]: "_");
    }
    printf("\n");
    return 0;
}

void list_tasks(sqlite3 *db)
{
    char *err_msg = 0;
    const char *sql = "SELECT * FROM Tasks;";
    int rc = sqlite3_exec(db, sql, list_callback, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to list tasks: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

int main()
{
    sqlite3 *db;
    int rc;

    initialize_db();

    rc = sqlite3_open("todo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error opening: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    add_task(db, "add_task function", "programming", "01/17/2024", "01/18/2024", "01/18/2024", "Completed",NULL, NULL);
    list_tasks(db);
    

    sqlite3_close(db);

    return 0;
}
