#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char *name;
} Task;

void initialize_db()
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc;
    const char *sql;

    rc = sqlite3_open("tiny-todo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    sql = "CREATE TABLE IF NOT EXISTS Tasks("
                      "Id INTEGER PRIMARY KEY, "
                      "Name TEXT NOT NULL); ";

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

    sql = "INSERT INTO Tasks (Name) VALUES (?);";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    const char *task_data[] = {task.name};

    sqlite3_bind_text(stmt, 1, task_data[0] ? task_data[0] : NULL, -1, SQLITE_TRANSIENT);

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
        const char *colText = (const char *)sqlite3_column_text(stmt, 0);
        if (colText) {
            char *temp = malloc(strlen(colText) + 1);
            if (temp) {
                strcpy(temp, colText);
            }
            *((char **)&task) = temp;
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
    };

    sqlite3_stmt *stmt;
    int rc;
    const char *sql = "UPDATE Tasks SET Name = ? WHERE Id = ?;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, task_fields[0], -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, task_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Task updated successfully\n");
    }

    sqlite3_finalize(stmt);

    free(*((char **)&current_task));
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

typedef struct {
    Task *tasks;
    size_t count;
} TaskList;

TaskList fetch_tasks(sqlite3 *db) {
    TaskList tasklist = {NULL, 0};
    sqlite3_stmt *stmt;
    const char *sql;

    sql = "SELECT Id, Name, Category, StartDate, DueDate, CompletionDate, Status, Priority, Description FROM Tasks;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return tasklist;
    }

    size_t capacity = 10;
    tasklist.tasks = malloc(capacity * sizeof(Task));
    if (!tasklist.tasks) {
        fprintf(stderr, "Failed to allocate memory\n");
        sqlite3_finalize(stmt);
        return tasklist;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (tasklist.count >= capacity) {
            capacity *= 2;
            Task *temp = realloc(tasklist.tasks, capacity * sizeof(Task));
            if (!temp) {
                fprintf(stderr, "Failed to realloc memory\n");
                break;
            }
            tasklist.tasks = temp;
        }

        Task *task = &tasklist.tasks[tasklist.count++];
        task->id = sqlite3_column_int(stmt, 0);
        task->name = strdup((const char *)sqlite3_column_text(stmt, 1));
    }

    sqlite3_finalize(stmt);
    return tasklist;
}

int main()
{
    sqlite3 *db;
    int rc;

    // const int screenWidth = 800;
    // const int screenHeight = 450;
    // Color background_color = RAYWHITE;

    initialize_db();

    rc = sqlite3_open("tiny-todo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error opening: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    
    // TaskList tasklist = fetch_tasks(db);

    // InitWindow(screenWidth, screenHeight, "Raylib test");

    // SetTargetFPS(60);

    // while (!WindowShouldClose()) {
    //     BeginDrawing();
    //         ClearBackground(background_color);
    //         if (GuiButton((Rectangle){ 10, 40, 120, 30}, "Click Me")) {
    //             background_color = RED;
    //         }
    //         DrawText("Welcome", 190, 200, 20, LIGHTGRAY);
    //     EndDrawing();
    // }

    // CloseWindow();
    Task newTask = {
        .name = "TaskName",
    };

    add_task(db, newTask);
    add_task(db, newTask);

    Task updateTask = {
        .name = "testing_new_edit",
    };

    edit_task(db, 1, updateTask);
    
    list_tasks(db);

    // for (size_t i = 0; i < tasklist.count; i++) {
    //     Task *task = &tasklist.tasks[i];
    //     printf("Task %zu: \n", i + 1);
    //     printf("ID: %d\n", task->id);
    //     printf("Name: %s\n", task->name);
    //     printf("Category: %s\n", task->category);
    //     printf("Start Date: %s\n", task->start_date);
    //     printf("Due Date: %s\n", task->due_date);
    //     printf("Completion Date: %s\n", task->completion_date);
    //     printf("Status: %s\n", task->status);
    //     printf("Priority: %s\n", task->priority);
    //     printf("Description: %s\n\n", task->description);
    // }

    // for (size_t i = 0; i < tasklist.count; i++) {
    //     free(tasklist.tasks[i].name);
    //     free(tasklist.tasks[i].category);
    //     free(tasklist.tasks[i].start_date);
    //     free(tasklist.tasks[i].due_date);
    //     free(tasklist.tasks[i].completion_date);
    //     free(tasklist.tasks[i].status);
    //     free(tasklist.tasks[i].priority);
    //     free(tasklist.tasks[i].description);
    // }
    // free(tasklist.tasks);

    sqlite3_close(db);

    return 0;
}
