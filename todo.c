#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

const char* get_column_text(sqlite3_stmt *stmt, int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    return text ? (const char*)text : NULL;
}

Task get_task_by_id(sqlite3 *db, int task_id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT Name, Category, StartDate, DueDate, CompletionDate, Status, Priority, Description FROM Tasks WHERE Id = ?;";
    Task task = {0};

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, task_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        for (int i = 0; i < NUM_OF_COLS; i++) {
            const char *colText = (const char *)sqlite3_column_text(stmt, i);
            if (colText) {
                char *temp = malloc(strlen(colText) + 1);
                if (temp) {
                    strcpy(temp, colText);
                }
                *((char **)&task + i) = temp;
            }
        }
    }

    sqlite3_finalize(stmt);
    return task;
}

void edit_task(sqlite3 *db, int task_id, Task updated_task) {
    Task current_task;
    
    current_task = get_task_by_id(db, task_id);

    const char* task_fields[] = {
        updated_task.name ? updated_task.name : current_task.name,
        updated_task.category ? updated_task.category : current_task.category,
        updated_task.start_date ? updated_task.start_date : current_task.start_date,
        updated_task.due_date ? updated_task.due_date : current_task.due_date,
        updated_task.completion_date ? updated_task.completion_date : current_task.completion_date,
        updated_task.status ? updated_task.status : current_task.status,
        updated_task.priority ? updated_task.priority : current_task.priority,
        updated_task.description ? updated_task.description : current_task.description
    };

    sqlite3_stmt *stmt;
    int rc;
    const char *sql = "UPDATE Tasks SET Name = ?, Category = ?, StartDate = ?, DueDate = ?, CompletionDate = ?, Status = ?, Priority = ?, Description = ? WHERE Id = ?;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    for (int i = 0; i < 8; i++) {
        sqlite3_bind_text(stmt, i + 1, task_fields[i], -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_int(stmt, 9, task_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Task updated successfully\n");
    }

    sqlite3_finalize(stmt);

    for (int i = 0; i < NUM_OF_COLS; i++) {
        free(*((char **)&current_task + i));
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

    const int screenWidth = 800;
    const int screenHeight = 450;
    
    initialize_db();

    rc = sqlite3_open("todo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error opening: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    InitWindow(screenWidth, screenHeight, "Raylib test");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Welcome", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    // Task newTask = {
    //     .name = "TaskName",
    //     .due_date = "01-20-2024",
    //     .description = "Sample Task Description",
    // };

    // add_task(db, newTask);

    list_tasks(db);

    Task updateTask = {
        .name = "testing_new_edit",
        .priority = "low",
        .category = "programming",
    };

    edit_task(db, 4, updateTask);
    
    list_tasks(db);

    sqlite3_close(db);

    return 0;
}
