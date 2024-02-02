ironic, isn't it? a to-do list for creating a to-do program.
tech used:

- C
- sqlite3
- some GUI creator (raylib / raygui tbd)

Todo

- choose resources
- make a database
- add categories / column to sql
- create add task function
- create list task function
- create delete task function
- create edit task function
- create get task from ID function
- make edit task function pull from ID to update (removes required params)
- add raylib / raygui

- --> create clear database function (hard reset) + WARNING
- --> print / format task table

///////////////

- make UI
- add task button
- task fields
- edit task button
- delete task button
- hard reset button + confirm

02/02/2024 Update:
The SQL db is working well, and the add/edit/delete functions are working as well.

Tried to create a tasklist struct to store the tasks in dynamic memory (so that I can iterate over it in the future for the GUI). The implementation is off somehow, and it's currently segfaulting - will need to isolate.

Thinking about making a smaller copy of the program to have less complexity (less columns, less copying code, etc). Then I'll try to find out exactly where it's breaking.
