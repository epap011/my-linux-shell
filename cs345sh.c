/******************************************************
 * @author Efthymios Papageorgiou : csd4340@csd.uoc.gr
 * @date   Friday,  5 November 2021 
 ******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_COMMANDS 128
#define MAX_COMMAND_ARGUMENTS 64
#define COMMAND_LENGTH 2048

char*  read_line();
char** parse_line(char* line);
int    execute_command(char* command);
int    execute_simple(char** command);
int    execute_redirection(char** exec, char* redirection_type, char* file_name);
int    execute_pipe(char* command);
int    cd_command(const char* path);
int    exit_command();
int    help_command();
int    init_string_array(char** array, int size);
int    remove_quotes(char* str);
int    empty_string_array(char** array, int size);
int    count_pipes(char* array);

int main() {

    char *cwd, *line, **commands;
    int status, cwd_length = 256, i, command_index;

    cwd = (char*)malloc(sizeof(char) * cwd_length);
    while(1) {
        printf("%s@cs345sh/%s$ ", getlogin(), getcwd(cwd, cwd_length));
        
        line = read_line();          //Phase 1
        commands = parse_line(line); //Phase 2


        //Phase 3: One Execution / Command
        for(command_index = 0; command_index < MAX_COMMANDS; command_index++) {         
            if(commands[command_index][0] == '\0') break;
            if((status = execute_command(commands[command_index])) == 0) break;
        }

        for(i = 0; i < MAX_COMMANDS; i++) free(commands[i]);
        free(commands);

        //Exit is the only commmand, returns zero
        if(status == 0) break;
    }

    free(cwd);
    return 0;
}

/**
 * This function returns a line, without 
 * the delimiter character, from stdin.
 **/
char* read_line() {

    char* command_line_buffer       = NULL;
    size_t command_line_buffer_size = 0;
    int chars;

    if(chars = getline(&command_line_buffer, &command_line_buffer_size, stdin)) {
        if(command_line_buffer[chars-1] == '\n') command_line_buffer[chars-1] = '\0';
        if(feof(stdin)) exit(EXIT_SUCCESS);
    }

    return command_line_buffer;
}

/**
 * This function translates a line to an array of line-commands.
 * Each row represents a command with the execution arg
 * as the first argument and the arguments.
 **/
char** parse_line(char* line) {
    
    int line_length, command_line_index = 0, i, j, k;
    char** command_lines; 

    //creation
    command_lines = (char**)malloc(sizeof(char*) * MAX_COMMANDS);
    for(i = 0; i < MAX_COMMANDS; i++) {
        command_lines[i] = (char*)malloc(sizeof(char) * COMMAND_LENGTH);
    }

    //initialization
    for(i = 0; i < MAX_COMMANDS; i++) {
        for(j = 0; j < COMMAND_LENGTH; j++) {
            command_lines[i][j] = '\0';
        }
    }

    //Seperate multiple commands, seperated by ';'
    line_length = strlen(line);
    int non_whitespace_char_found                 = 0;
    int last_non_whitespace_char_before_semicolon = -1;
    
    j = 0;
    for(i = 0; i < line_length; i++){
        if(line[i] == ' ' && non_whitespace_char_found == 0) continue;
        if(line[i] == ';') {
            //Replace whitespaces before ';' with the null terminator character
            if(last_non_whitespace_char_before_semicolon != -1) {
                for(k = last_non_whitespace_char_before_semicolon; k < i; k++) {
                    command_lines[command_line_index][k] = '\0';
                }
            }

            if(i == line_length-1) break; //there is no following command
            command_line_index++;
            non_whitespace_char_found = 0;
            last_non_whitespace_char_before_semicolon = -1;
            j = 0;
            continue; 
        }
        if(line[i] == ' ' && last_non_whitespace_char_before_semicolon == -1) last_non_whitespace_char_before_semicolon = j;
        if(line[i] != ' ') last_non_whitespace_char_before_semicolon = -1;
        command_lines[command_line_index][j] = line[i];
        non_whitespace_char_found = 1;
        j++;
    }

    //useless whitespaces chars found until the end of the line
    //if the last char was ';' it could be fixed on the above for-loop
    if(last_non_whitespace_char_before_semicolon != -1) {
        for(k = last_non_whitespace_char_before_semicolon; k < line_length; k++) {
            command_lines[command_line_index][k] = '\0';
        }
    }

    return command_lines;
}

/**
 * This function is one step before the execution.It takes one command and
 * checks what type this command is.Multi - redirection, multi - piping and
 * simple command execution are supported.
 */
int execute_command(char* command) { 

    char* arg_list[MAX_COMMAND_ARGUMENTS], command_original_form[COMMAND_LENGTH], *arg;
    int args_counter = 0, redirection_flag = 0, pipes = 0, echo_exists = 0;

    init_string_array(arg_list, MAX_COMMAND_ARGUMENTS);
    strcpy(command_original_form, command);

    arg = strtok(command, " ");
    if(strcmp(arg, "echo") == 0) echo_exists = 1;
    arg_list[args_counter++] = arg;
    while((arg = strtok(NULL, " ")) != NULL) { 
        
        //multi - redirection support     
        if(strcmp(arg, "<") == 0 || strcmp(arg, ">") ==  0 || strcmp(arg, ">>") == 0) { 
            redirection_flag = 1;
            if((execute_redirection(arg_list, arg, strtok(NULL, " "))) == -1) return -1;
            continue;
        }

        //pipe - support
        if(strcmp(arg, "|") == 0) {
            pipes = 1;
            execute_pipe(command_original_form);            
            break;
        }

        //The first argument of echo
        if(echo_exists == 1) remove_quotes(arg);
        
        arg_list[args_counter++] = arg;   
    }

    //cd, exit and help considered built-in commands of the shell
    if(strcmp(arg_list[0], "cd") == 0) {
        if(args_counter == 2) cd_command(arg_list[1]);
        else fprintf(stderr, "cs345sh: cd: expected path\n");
    } else
    if(strcmp(arg_list[0], "exit") == 0) {
        if(args_counter == 1) return exit_command();
        else fprintf(stderr, "cs345sh: exit: too much arguments\n");
    } else
    if(strcmp(arg_list[0], "help") == 0) {
        if(args_counter == 1) help_command();
        else fprintf(stderr, "cs345sh: help: too much arguments\n");
    }
    //simple non-built-commands
    else {
        if(redirection_flag == 0 && pipes == 0) execute_simple(arg_list);   
    }
    
    return 1;
}

/**
 * This function executes a command with no pipeling/redirection
 **/
int execute_simple(char** command) {
    
    pid_t pid;
    int status;
    
    pid = fork();
    if(pid > 0) { //Parent Process
        do {
            waitpid(pid, &status, WUNTRACED);
        }while (!WIFEXITED(status) && !WIFSIGNALED(status));    
    } else
    if(pid == 0) { //Child Process
        execvp(command[0], command);
    }
    else { //fork() failed
        fprintf(stderr, "cs345: error: fork() failed!\n");
        return -1;
    }
    return 1;
}

/**
 * This function executes a command with redirection in "mind".
 * Warning! Redirection symbol must be between one
 * left and one right whitespace e.g cat < a.txt
 **/
int execute_redirection(char** exec, char* redirection_type, char* file_name){

    pid_t pid, wpid;
    int fd, status;

    pid = fork();
    if(pid > 0) { //Parent Process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        }while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else
    if(pid == 0) { //Child Process
        if(strcmp(redirection_type, "<")  ==  0) {
            if((fd = open(file_name, O_RDONLY, 0777)) == -1) {
                fprintf(stderr, "error: open() failed!\n");
                return -1;
            }
            dup2(fd, STDIN_FILENO);  
        } else
        if(strcmp(redirection_type, ">")  ==  0) {
            if((fd = open(file_name, O_WRONLY | O_CREAT, 0777)) == -1) {
                fprintf(stderr, "error: open() failed!\n");
                return -1;  
            }
            dup2(fd, STDOUT_FILENO);
        } else
        if(strcmp(redirection_type, ">>") ==  0) {
            if((fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0777)) == -1) {
                fprintf(stderr, "error: open() failed!\n");
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
        }
       
        close(fd);
        execvp(exec[0], exec);     
    } 
    else { //fork() failed
        fprintf(stderr, "cs345: error: fork() failed!\n");
        return -1;
    }

    return 1;
}

/**
 * This function takes a whole command which is sure that it contains at least
 * one pipe and executes it "pipe by pipe".Warning! Pipe must be between one
 * left and one right whitespace e.g cat a.txt | sort.
 **/
int execute_pipe(char* command) {
    
    pid_t pid;
    int *fds, status, pipe_counter, cmd_counter, args_counter, i;
    char* arg_list[MAX_COMMAND_ARGUMENTS], *arg;

    init_string_array(arg_list, MAX_COMMAND_ARGUMENTS);
    pipe_counter = count_pipes(command);
    fds = (int*)malloc(sizeof(int) * pipe_counter * 2);
    
    for(i = 0; i < pipe_counter; i++) {
        if(pipe(fds + i*2) == -1){
            fprintf(stderr, "cs345sh: error: pipe() failed!\n");
            return -1;
        }
    }

    cmd_counter = 0;
    while(1) {
        args_counter = 0;
        if(cmd_counter == 0) {
            arg = strtok(command, " ");
            arg_list[args_counter++] = arg;
        }
        
        while((arg = strtok(NULL, " ")) != NULL) {
            if(strcmp(arg, "|") == 0) break;
            arg_list[args_counter++] = arg;   
        }
        
        pid = fork();
        if(pid == 0) { //Child Process
            if(cmd_counter != 0) { //if NOT the first command
                dup2(fds[(cmd_counter-1)*2], STDIN_FILENO);       
            }
            if(cmd_counter != pipe_counter) { //if NOT the last  command 
                dup2(fds[cmd_counter*2+1], STDOUT_FILENO); 
            }
            for(i = 0; i < pipe_counter * 2; i++) close(fds[i]);

            execvp(arg_list[0], arg_list);
        }
        else
        if(pid < 0) { //fork() failed
            fprintf(stderr, "cs345: error: fork() failed!\n");
            return -1;
        }
        if(pid > 0) {
            cmd_counter++;
            if(arg == NULL) break;
        }       
    } 
    
    for(i = 0; i < pipe_counter * 2; i++) close(fds[i]);
    for(i = 0; i < pipe_counter + 1; i++) wait(&status);
    
    return 1;
}

/****Shell Built-in function****/
int cd_command(const char* path) {
    
    if(chdir(path) == -1) {
        fprintf(stdout, "cs345sh: cd: %s: No such file or directory\n", path);
        return -1;
    }
    
    return 1;
}

int exit_command() {
    return 0;
}

int help_command() {
    printf("#############################\n");
    printf("#        CS345SHell         #\n");
    printf("# By Efthymios Papageorgiou #\n");
    printf("#############################\n");
    
    printf("\n***************Restrictions***************\n");
    printf("1. The maximum number of commands seperated by ';' are 128\n");
    printf("2. The maximum number of arguments of each command are 64\n");
    printf("3. The maximum length of a line is 2048 bytes\n");

    printf("\n***************RULES***************\n");
    printf("Rule 1: Redirection works only between 2 spaces e.g cat > file1.txt >> file2.txt\n");
    printf("Rule 2: Pipes works only between 2 spaces e.g ls | grep \"root\" | wc -w\n");

    return 1;
}

/****Utility Functions****/
int init_string_array(char** array, int size) {

    int i;
    for(i = 0; i < size; i++) array[i] = NULL;

    return 1;
}

int empty_string_array(char** array, int size) {
    
    int i;
    for(i = 0;  i < size; i++) {
        array[i] = NULL;
    }
}

int count_pipes(char* array) {

    int pipes_counter = 0, i;
    for(i = 0; array[i] != '\0'; i++) {
        if(array[i] == '|')
            pipes_counter++;
    }
    return pipes_counter;
} 

int remove_quotes(char* str) {
    
    int len, i;

    len = strlen(str);
    if(str[0] == '"') {
        for(i = 1; i < len; i++)
            str[i - 1] = str[i];
        str[len - 1] = '\0';
        if(str[len-2] == '"') 
            str[len-2] = '\0';   
    } else
    if(str[len-1] == '"') str[len-1] = '\0';
    else return -1;
    
    return 1;
}