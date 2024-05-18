//Peanut Shell: simple Shell program
//By Adrian Faircloth
//5-16-2024

#include <filesystem>
#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include <ctime>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
namespace fs = std::filesystem;
using namespace std;

int cd(char** args, int &argc);
int ls(char** args, int &argc);
int help(char** args, int &argc);
int date(char** args, int &argc);
int mkdir(char** args, int &argc);
int touch(char** args, int &argc);
int rm(char** args, int &argc);
int exit(char** args, int &argc);

char* builtin[] = {
    (char*)"cd",
    (char*)"ls",
    (char*)"help",
    (char*)"date",
    (char*)"mkdir",
    (char*)"touch",
    (char*) "rm",
    (char*)"exit"
};

//Pointers to builtin functions
int (*builtin_funcs[]) (char**, int&) {
    &cd,
    &ls,
    &help,
    &date,
    &mkdir,
    &touch,
    &rm,
    &exit
};
int builtin_count = sizeof(builtin) / sizeof(char**);

//Builtin command: change directory
int cd(char** args, int &argc) {
    fs::path fpath = fs::current_path();
    if (args[1] == NULL) {
        cout << "Peanut: Expected argument for cd\n";
    }
    else {
        try {
            fs::current_path(args[1]);
            cout << "Successfully moved to " << fs::current_path().string() << endl;
        }
        catch (fs::filesystem_error) {
            perror("Peanut");
        }
    }
    return 1;
}

//Builtin command: print current directory contents
int ls(char** args, int &argc) {
    cout << "Directory: " << fs::current_path().string() << endl;
    for (auto &file : fs::directory_iterator(fs::current_path())) {
        cout << "\t" << file << endl;
    }
    return 1;
}

//Builtin command: print help
int help(char** args, int &argc) {
    cout << "**Peanut Shell \n";
    cout << "**Enter program names and args to run\n";
    cout << "**The following are builtin commands: \n";
    for (int i = 0; i < builtin_count; i++) {
        cout << "\t" << builtin[i] << "\n";
    }
    return 1;
}

//Builtin command: Print date and time
int date(char** args, int &argc) {
    const time_t time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    cout << ctime(&time);
    return 1;
}

//Builtin command: Create directory of given name
int mkdir(char** args, int &argc) {
    fs::create_directory(args[1]);
    return 1;
}

//Builtin command: Creates file(s) with given name(s)
//Opens & closes given filename, creates file if it does not exist
int touch(char** args, int &argc) {
    if (args[1] == NULL) {
        cout << "Expected argument for touch\n";
        return 1;
    }

    string s;

    for (int i = 1; i < argc; i++) {
        s= args[i];
        ofstream{s};
    }
    return 1;
}

//Builtin command: removes file or directory
// -r option recursively removes directory and all its contents
int rm(char** args, int &argc) {
    if (!strcmp(args[1], "-r")) {
        if (argc < 3) {
            cout << "Expected argument for rm\n";
            return 1;
        }
        if (!fs::remove_all(args[2])) {
            perror("Peanut");
        }
        return 1;
    }
    if (!fs::remove(args[1])) {
        perror("Peanut");
    }
    return 1;
}

//Builtin command: exit program
int exit(char** args, int &argc) {
    return 0;
}

//Runs program and waits for it to terminate
//Uses forked child process to try and execute program from args
int shell_run(char** args) {
    int status;
    pid_t pid, wpid;
    if ((pid = fork()) == 0) {
        //Child process
        if (execvp(args[0], args) == -1) {
            perror("Peanut");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        //Forking error
        perror("Peanut");
    }
    else {
        //Parent process
        wait(NULL);
    }
    return 1;

}

//Executes shell input
//Checks if arg matches any builtins, else runs general execute command
int shell_execute(char** args, int argv) {
    if(args[0] == NULL) {
        cout << "No command entered.\n";
        return 1;
    }
    for (int i = 0; i < builtin_count; i++) {
        if (strcmp(args[0], builtin[i]) == 0) {
            return builtin_funcs[i](args, argv);
        }
    }
    return shell_run(args);
}

//Reads in line and converts to C-style string
char* shell_readline() {
    string line = "";
    getline(cin, line);
    int length = line.length();
    char* line_arr = new char[length+1];
    strcpy(line_arr, line.c_str());
    return line_arr;
}

#define BUFFSIZE 64;
#define DELIMS " \t\n\a\r"
//Takes C-style string and tokenizes, splitting at any whitespace
//Dynamically reallocates memory if needed
//Returns C-style string array
char** shell_splitline(char* line, int &argc) {
    int buffsize = BUFFSIZE;
    int position = 0;
    char** tokens = (char**) malloc(buffsize * sizeof(char*));
    char** tokens_backup;
    char* token;

    if (!tokens) {
        cout << "Peanut: Allocation error\n";
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIMS);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        argc++;
        if (position >= buffsize) {
            buffsize += BUFFSIZE;
            tokens_backup = tokens;
            tokens = (char**) realloc(tokens, buffsize * sizeof(char*));

            if (!tokens) {
                free(tokens_backup);
                cout << "Peanut: Allocation error\n";
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIMS);
    }
    tokens[position] = NULL;
    return tokens;
}

//Loop for getting input, parsing, and executing
void shell_loop() {
    char *input;
    char **args;
    int argc;
    int status = 1;
    while(status) {
        argc = 0;
        cout << "Peanut> ";
        input = shell_readline();
        args = shell_splitline(input, argc);
        status = shell_execute(args, argc);
    }
}

//Clears terminal and runs shell loop
int main(int argc, char** argv) {
    cout << "\033[2J\033[1;1H";
    shell_loop();
    return 0;
}