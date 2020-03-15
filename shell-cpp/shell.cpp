#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


// struct for holding a command and its parameters
struct command {
    char *proc;
    int argc;
    char **argv;
    char *stdin;
    char *stdout;

    command() {
        proc = NULL;
        stdin = NULL;
        stdout = NULL;
    } 

    command(int argc): command() {
        this->argc = argc;
        argv = new char*[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = NULL;
        }
    }

    ~command() {
        if (proc != NULL) delete proc;
        if (stdin != NULL) delete stdin;
        if (stdout != NULL) delete stdout;
        // start from 1st since 0th is already proc name and freed
        for (int i = 1; i < argc; ++i) {
            if (argv[i] != NULL) delete argv[i];
        }
        delete[]argv;
    }

};

// struct for containing an entire command sequence
struct pipeline {
    command **cmds;
    int count;
    
    pipeline(int count) {
        this->cmds = new command*[count];
        this->count = count;
    }

    ~pipeline() {
        for (int i = 0; i < count; ++i) {
            if (cmds[i] != NULL) delete cmds[i];
        }
        delete[]cmds;
    }
};

// counts the occurences for a character
int counter(char *input, char delim) {
    int count = 0;
    for (int i = 0; input[i] != '\0'; ++i) {
        if (input[i] == delim) {
            count++;
        }
    }
    return count;
}

// uses strtok_r to tokenize and return deep-copies of the tokens
char* tokenize(char *input, const char *delim, char **rest) {
    char *token = strtok_r(input, delim, rest);
    if (token == NULL) {
        return NULL;
    }
    char *new_token = new char[strlen(token)];
    strcpy(new_token, token);
    return new_token;
}

// searches the PATH variable to find a process
bool find_process(char *proc, char *fullPath) {
    // get and deep-copy the PATH variable
    char *pathenv = getenv("PATH");
    char *path = new char[strlen(pathenv) + 1];
    // append : to create an empty token (for absolute/relative path inputs)
    strcpy(path, ":");
    // append the rest of the pathenv
    strcat(path, pathenv);

    char *rest = NULL;
    char *token = tokenize(path, ":", &rest);
    
    bool found = false;

    // build the entire path for each entry in PATH
    while (token != NULL && !found) {
        strcpy(fullPath, "");
        strcat(fullPath, token);
        strcat(fullPath, "/");
        strcat(fullPath, proc);
        // check if the executable exists
        if (access(fullPath, F_OK) == 0) {
            found = true;
        }
        token = tokenize(NULL, ":", &rest);
    }

    delete[]path;
    return found;
}

command* make_command(char input[]) {
    int totalArgs = counter(input, ' ') + 2;
    char *rest = NULL;
    char *token = tokenize(input, " ", &rest);

    command *cmd = new command(totalArgs);

    int i = 0, j = 0;
    bool stdin = false, stdout = false;
    while (token != NULL) {
        if (i == 0) {
            cmd->proc = token;
            cmd->argv[j] = token;
            j++;
        } else if (token[0] == '<') {
            if (strlen(token) > 1) {
                cmd->stdin = strtok(token, "<");
            } else {
                stdin = true;
            }
        } else if (token[0] == '>') {
            if (strlen(token) > 1) {
                cmd->stdout = strtok(token, ">");
            } else {
                stdout = true;
            }
        } else if (stdin) {
            cmd->stdin = token;
            stdin = false;
        } else if (stdout) {
            cmd->stdout = token;
            stdout = false;
        } else {
            cmd->argv[j] = token;
            j++;
        }

        token = tokenize(NULL, " ", &rest);
        i++;
    }
    return cmd;
}

pipeline* make_pipeline(char input[]) {
    int totalCommands = counter(input, '|') + 1;
    pipeline *pipe = new pipeline(totalCommands);
 
    char *rest = NULL;
    char *token = tokenize(input, "|", &rest);

    int i = 0;
    while (token != NULL) {
        pipe->cmds[i] = make_command(token);
        token = tokenize(NULL, "|", &rest);
        i++;
    }

    return pipe;
}

void close_pipes(int* pipes) {
    close(pipes[0]);
    close(pipes[1]);
}

void execute_pipeline(pipeline* p) {

    // create pipes    
    int *pipes = new int[2 * p->count];
    for (int i = 0; i < p->count; ++i) {
        pipe(pipes + i * 2);
    }

    int pid;

    for (int i = 0; i < p->count; ++i) {
        command *cmd = p->cmds[i];
        pid = fork();

        if (pid == 0) {
            if (cmd->stdin != NULL) {
                int fd = open(cmd->stdin, O_RDONLY);
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (cmd->stdout != NULL) {
                int fd = open(cmd->stdout, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            // if commands are piped
            if (p->count > 1) {
                
                // if not first, redirect stdin
                if (i > 0) {
                    dup2(pipes[(i - 1) * 2], STDIN_FILENO);
                }

                // if not last, redirect stdout
                if (i < p->count - 1) {
                    dup2(pipes[i * 2 + 1], STDOUT_FILENO);
                }
            }

            // close all pipes for this process
            for (int i = 0; i < p->count; ++i) {
                close(pipes[i * 2]);
                close(pipes[i * 2 + 1]);
            }
            
            char fullPath[200];
            find_process(cmd->proc, fullPath);
            // exec this command
            execv(fullPath, cmd->argv);
        }

    }

    // close all pipes for parent process
    for (int i = 0; i < p->count; ++i) {
        close(pipes[i * 2]);
        close(pipes[i * 2 + 1]);
    }

    delete[]pipes;

    // wait for last spawned process
    waitpid(pid, NULL, 0);
}

int main() {
    char input[200];
    while (true) {
        std::cout << "Command >";
        std::cin.getline(input, 200);
        if (strcmp(input, "exit") == 0) {
            break;
        }
        pipeline *pipe = make_pipeline(input);
        execute_pipeline(pipe);
        delete pipe;
    }

    return 0;
}
