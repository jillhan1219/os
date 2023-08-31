#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_ARGS 10

void execute_command(char **args, int pipe_read_fd) {
    pid_t pid = fork();
    if (pid == 0) {
        if (pipe_read_fd != -1) {
            dup2(pipe_read_fd, STDIN_FILENO);
            close(pipe_read_fd);
        }
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork failed");
    }
}

int parse_command(char *input, char ***args) {
    int arg_count = 0;
    char *token;
    char *rest = input;

    while ((token = strtok_r(rest, " \n", &rest))) {
        if (strcmp(token, "|") == 0) {
            // Found pipe symbol, end current command
            (*args)[arg_count] = NULL;
            arg_count = 0;
            execute_command(*args, -1);
            pipe(pipe_fd);
            pipe_read_fd = pipe_fd[0];
        } else {
            // Add argument to current command
            (*args)[arg_count++] = token;
            if (arg_count == MAX_ARGS - 1) {
                break;
            }
        }
    }

    // End last command
    if (arg_count > 0) {
        (*args)[arg_count] = NULL;
        execute_command(*args, pipe_read_fd);
    }

    return 0;
}

int main() {
    int server_socket_fd, client_socket_fd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length = sizeof(client_address);
    char input[256];
    char *args[MAX_ARGS];
    int arg_count = 0;
    int pipe_fd[2] = {-1, -1};
    int pipe_read_fd = -1;

    printf("Starting server...\n");

    // Create server socket
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    // Bind server socket to server address
    if (bind(server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for client connections
    if (listen(server_socket_fd, 5) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is ready and listening on port 8080...\n");

    while (1) {
        // Accept client connection
        client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket_fd == -1) {
            perror("accept failed");
            continue;
        }

        printf("Accepted new client connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Fork child process to handle client communication
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(server_socket_fd);

            // Set client socket as


------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_ARGS 10
#define MAX_COMMANDS 10
#define BUFFER_SIZE 1024

int execute_command(char **args, int in, int out) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        if (in != STDIN_FILENO) {
            dup2(in, STDIN_FILENO);
            close(in);
        }
        if (out != STDOUT_FILENO) {
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        execvp(args[0], args);
        perror(args[0]);
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return status;
    }
}

int execute_pipeline(char **commands[], int n) {
    int pipes[MAX_COMMANDS-1][2];
    for (int i = 0; i < n-1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }
    int in = STDIN_FILENO;
    for (int i = 0; i < n; i++) {
        int out = STDOUT_FILENO;
        if (i < n-1) {
            out = pipes[i][1];
        }
        if (execute_command(commands[i], in, out) == -1) {
            return -1;
        }
        if (i < n-1) {
            close(pipes[i][1]);
            in = pipes[i][0];
        }
    }
    return 0;
}

int parse_input(char *input, char **args[]) {
    char *command_str;
    char *command;
    int i = 0;
    int j = 0;
    int k = 0;
    int n = 0;
    int len = strlen(input);
    while (i < len) {
        if (input[i] == '|' || input[i] == '\n') {
            command_str[j] = '\0';
            command = strtok(command_str, " ");
            args[k][0] = command;
            j = 0;
            k++;
            n++;
            if (input[i] == '|') {
                args[k-1][1] = NULL;
            }
        } else if (input[i] == ' ') {
            command_str[j] = '\0';
            command = strtok(command_str, " ");
            args[k][j] = command;
            j++;
        } else {
            command_str[j] = input[i];
            j++;
        }
        i++;
    }
    args[k][j] = NULL;
    return n;
}
/*
// Parse a command string into an array of arguments
int parse_command(char *command, char **args) {
    int argc = 0;
    char *token = strtok(command, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }

    args[argc] = NULL;  // Mark the end of the argument list with NULL
    return argc;
}
*/
// Count the number of pipes in a command string
/*
int count_pipes(char **command, int num_args) {
    int num_pipes = 0;
    for (int i = 0; i < num_args; i++) {
        if (strcmp(command[i], "|") == 0) {
            num_pipes++;
        }
    }
    return num_pipes;
}
*/
void handle_client(int sockfd) {
    char buffer[BUFFER_SIZE];
    int n;

    printf("Client connected.\n");

    while (1) {
        bzero(buffer, BUFFER_SIZE);//memset(buffer,0,MAX_BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE);

        if (n <= 0) {
            printf("Client disconnected.\n");
            close(sockfd);
            return;
        }

        buffer[n - 1] = '\0'; // remove newline character

        if (strcmp(buffer, "exit") == 0) {
            printf("Client requested to disconnect.\n");
            close(sockfd);
            return;
        }

        // Parse command and arguments
        char *command[MAX_ARGS][MAX_ARGS];
        int num_args = parse_input(buffer, command);//ok

        if (num_args > 0) {
            // Check for pipes
            //int num_pipes = count_pipes(command, num_args);
            if (num_args == 1) {
                // No pipes, execute single command
                execute_command(command[0],STDIN_FILENO, STDOUT_FILENO);
            } else {
                // Pipes present, execute pipeline of commands
                //execute_pipeline(sockfd, command, num_args, num_pipes);
                execute_pipeline(command,num_args);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    // Create server socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind server socket to port
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);

        // Accept incoming connection
        int connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen);
        if (connfd < 0) {
            perror("accept");
            continue;
        }

        // Handle client connection
        handle_client(connfd);
    }

    return 0;
}
