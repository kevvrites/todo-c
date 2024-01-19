#include <sqlite3.h>
#include <stdio.h>

#define NUM_OF_COLS 8

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
    const char *sql;

    rc = sqlite3_open("todo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    sql = "CREATE TABLE IF NOT EXISTS Tasks("
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
}

void add_task(sqlite3 *db, Task task)
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
    
    const char *task_data[] = {task.name, task.category, task.start_date, task.due_date, task.completion_date, task.status, task.priority, task.description};

    for (int i = 0; i < NUM_OF_COLS; i++) {
        sqlite3_bind_text(stmt, i + 1, task_data[i] ? task_data[i] : NULL, -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Task added successfully\n");
    }

}

void edit_tasks(sqlite3 *db, int task_id, Task updated_task)
{
    sqlite3_stmt *stmt;
    int rc;
    const char *sql;

    sql = "UPDATE Tasks SET Name = ?, Category = ?, StartDate = ?, DueDate = ?, CompletionDate = ?, Status = ?, Priority = ?, Description = ? WHERE Id = ?;";
    const char* task_fields[] = {
        updated_task.name,
        updated_task.category,
        updated_task.start_date,
        updated_task.due_date,
        updated_task.completion_date,
        updated_task.status,
        updated_task.priority,
        updated_task.description,
    };

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    for (int i = 0; i < NUM_OF_COLS; i++) {
        sqlite3_bind_text(stmt, i + 1, task_fields[i] ? task_fields[i] : NULL, -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_int(stmt, 9, task_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Task updated successfully\n");
    }

    sqlite3_finalize(stmt);
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

void delete_task(sqlite3 *db, int task_id)
{
    sqlite3_stmt *stmt;
    int rc;
    const char *sql;

    sql = "DELETE FROM Tasks WHERE Id = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, task_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Task deleted successfully\n");
    }

    sqlite3_finalize(stmt);
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

    Task newTask = {
        .name = "TaskName",
        .due_date = "2024-01-20",
        .description = "Sample Task Description",
    };

    list_tasks(db);

    Task updateTask = {
        .name = "SameName",
        .due_date = "01-20-2024",
        .description = "New task description",
    };

    edit_tasks(db, 3, updateTask);
    
    list_tasks(db);

    sqlite3_close(db);

    return 0;
}
