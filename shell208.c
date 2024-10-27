/*
Katherine Baker and Ella Visconti
Command shell for assignment 9 and 10 in CS208
Code partially borrowed from provided samples
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#define COMMAND_BUFFER_SIZE     102
#define MAX_COMMAND_SIZE        COMMAND_BUFFER_SIZE - 2
#define COMMAND_INPUT_SUCCEEDED 0
#define COMMAND_INPUT_FAILED    1
#define COMMAND_END_OF_FILE     2
#define COMMAND_TOO_LONG        3

int get_command(char *command_buffer, int buffer_size);
void execute_command(char *command_line);
void sigint_handler(int);

/* Prints help statement, takes in no parameters, and returns nothing.*/
void print_help_statement() {
    printf("Currently our shell supports the following features:\n");
    printf("1) help: prints the help statement, which you are reading now.\n");
    printf("2) exit: exits the shell.\n");
    printf("3) commands that execvp can execute. Try things like ls, ls -l -a, or wc -l file.txt.\n");
    printf("4) redirecting output to another file; try things like ls > myFile.txt.\n");
    printf("5) redirecting input from a file; try things like wc -l < myFile.txt.\n");
    printf("6) piping the output of one command to another. Try things like: cat myFile.txt | grep s.\n");
    printf("7) Control-C does not interrupt the shell. You must type exit to exit the shell.\n");
}

/* signal handler for SIGINT 
    returns nothing, takes in the signal as an int*/
void sigint_handler(int sig) {
    fprintf(stderr, "^C\nshell208$ ");
    fflush(stderr);
}

/* Takes in no parameters, and returns 0 upon successful execution.
    Contains the main infinite loop to run the shell.*/
int main()
{
    const char *prompt = "shell208$ ";
    char command_line[COMMAND_BUFFER_SIZE];

    //Handling signal interrupts
    if (signal(SIGINT, sigint_handler) != SIG_DFL) {
        fprintf(stderr, "\n");
    }

    while (1)
    {
        printf("%s", prompt);
        fflush(stdout);
        int result = get_command(command_line, COMMAND_BUFFER_SIZE);
        if (result == COMMAND_END_OF_FILE)
        {
            // stdin has reached EOF
            break;
        }
        else if (result == COMMAND_INPUT_FAILED)
        {
            fprintf(stderr, "There was a problem reading your command. Please try again.\n");
            break;
        }
        else if (result == COMMAND_TOO_LONG)
        {
            fprintf(stderr, "Commands are limited to length %d. Please try again.\n", MAX_COMMAND_SIZE);
        }
        else
        {
            execute_command(command_line);
        }
    }

    return 0;
}

/*
    Retrieves the next line of input from stdin (where, typically, the user
    has typed a command) and stores it in command_buffer.

    The newline character (\n, ASCII 10) character at the end of the input
    line will be read from stdin but not stored in command_buffer.

    The input stored in command_buffer will be \0-terminated.

    Returns:
        COMMAND_TOO_LONG if the number of chars in the input line
            (including the \n), is greater than or equal to buffer_size
        COMMAND_INPUT_FAILED if the input operation fails with an error
        COMMAND_END_OF_FILE if the input operation fails with feof(stdin) == true
        COMMAND_INPUT_SUCCEEDED otherwise

    Preconditions:
        - buffer_size > 0
        - command_buffer != NULL
        - command_buffer points to a buffer large enough for at least buffer_size chars
*/
int get_command(char *command_buffer, int buffer_size)
{
    assert(buffer_size > 0);
    assert(command_buffer != NULL);

    if (fgets(command_buffer, buffer_size, stdin) == NULL)
    {
        if (feof(stdin))
        {
            return COMMAND_END_OF_FILE;
        }
        else
        {
            return COMMAND_INPUT_FAILED;
        }
    }

    int command_length = strlen(command_buffer);
    if (command_buffer[command_length - 1] != '\n')
    {
        // Reading the rest of the input
        char ch = getchar();
        while (ch != '\n' && ch != EOF)
        {
            ch = getchar();
        }

        return COMMAND_TOO_LONG;
    }

    // remove the newline character and null terminate.
    command_buffer[command_length - 1] = '\0';
    return COMMAND_INPUT_SUCCEEDED;
}

/* Takes in a char * array of input from the command line.
    Returns nothing.
    Where input gets processed and specified commands get executed.
    Calls help and exit functions if necessary. 
    */
void execute_command(char *command_line)
{
    int need_help = strcmp(command_line, "help");
    if (need_help == 0) {
        print_help_statement();
        return;
    }
    int to_exit = strcmp(command_line, "exit");
    if (to_exit == 0) {
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Unable to fork successfully.");
        exit(1);
    }

    int fd[2]; //piping
    
    if (pid == 0)
    {
        /* Child */
        fflush(stdout);

        // Prep arguments
        char *command_line_token = strtok(command_line, " ");
        char *my_argv[10];
        int i;
        int carrot_index = 0;
        int pipe_index = 0;
        int backwards_carrot_index = 0;
        for (i = 0; command_line_token != NULL; i++) {
            my_argv[i] = &command_line_token[0] + '\0';
            if (*my_argv[i] == '>') {
                carrot_index = i;
            }
            if (*my_argv[i] == '|') {
                pipe_index = i;
            }
            if (*my_argv[i] == '<') {
                backwards_carrot_index = i;
            }
            command_line_token = strtok(NULL, " ");
        }
        my_argv[i] = NULL;
        
        
        // Execute the program
        if (my_argv[1] != NULL) {
            if (carrot_index != 0) { 
                fflush(stdout);
                const char *file_name = my_argv[carrot_index+1];
                int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Trouble opening file");
                    exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("Trouble dup2-ing to stdout");
                    close(fd); 
                    exit(1);
                }
                close(fd);
                fflush(stdout);
                my_argv[carrot_index] = '\0';
                my_argv[carrot_index+1] = '\0';
                execvp(my_argv[0], my_argv);
                exit(1);
            }

            if (backwards_carrot_index!=0){
                fflush(stdout);
                const char *file_name = my_argv[backwards_carrot_index+1];
                int fd = open(file_name, O_RDONLY);
                if (fd < 0) {
                    perror("Trouble opening file");
                    exit(1);
                }
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("Trouble dup2-ing to stdout");
                    close(fd); 
                    exit(1);
                }
                close(fd);
                fflush(stdout);
                my_argv[backwards_carrot_index] = '\0';
                my_argv[backwards_carrot_index+1] = '\0';
                execvp(my_argv[0], my_argv);
                exit(1);

            }
        }

        
        if (pipe_index != 0) {
            
            char *first_half_pipe_array[10];
            char *second_half_pipe_array[10];
            //splitting pipe array
            int j;
            int k = 0;
            for (j = 0; my_argv[j] != NULL; j++) {
                if (j < pipe_index) {
                    first_half_pipe_array[j] = my_argv[j];
                }
                else if (j > pipe_index) {
                    second_half_pipe_array[k] = my_argv[j];
                    k++;
                }
            }
            first_half_pipe_array[pipe_index] = NULL;
            second_half_pipe_array[k] = NULL;
                
            if (pipe(fd) < 0) {
                perror("Unable to create pipe.");
                exit(1);
            }
            //create grandchild and do piping
            pid_t pid2= fork();

            if (pid2 < 0 ) {
                perror("Trouble forking");
                exit(1);
            }
            if (pid2 != 0) { //part of child
                close(fd[1]);
                if (dup2(fd[0], STDIN_FILENO) == -1) {
                    perror("Trouble redirecting stdin");
                    exit(1);
                }
                close(fd[0]);
                execvp(second_half_pipe_array[0], second_half_pipe_array);
                perror("execvp in child failed");
                exit(1);
                
            }
            else { //grandchild
                close(fd[0]);
                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("Trouble redirecting stdout");
                    exit(1);
                }
                close(fd[1]);
                execvp(first_half_pipe_array[0], first_half_pipe_array);
                perror("execvp in grandchild failed");
                exit(1);
            }
        }

        execvp(my_argv[0], my_argv);

        // Check for errors
        perror("exec failed"); 
        exit(1);
        fflush(stdout);
    }
    else
    {

        close(fd[0]);
        close(fd[1]);

        // Wait for the child to finish execution
        int status;
        wait(&status);
        wait(&status);
        fflush(stdout);
    }

}