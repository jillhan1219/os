#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_ARGS 10  // 最大参数个数
#define MAX_CMDS 10  // 最大命令个数
#define MAX_CMD_LEN 100  // 最大命令长度
#define MAX_CLIENTS 10  // 最大客户端个数
#define BUFFER_SIZE 1024  // 缓冲区大小

void handle_connection(int sock);
void handle_command(char *cmd, int sock);

int main(int argc, char *argv[]) {
    int port = atoi(argv[1]);  // 从命令行参数获取端口号
    int server_sock, client_sock, c, pid;
    struct sockaddr_in server, client;

    // 创建socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 绑定地址和端口
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 监听连接请求
    listen(server_sock, MAX_CLIENTS);

    // 处理连接请求
    c = sizeof(struct sockaddr_in);
    while (1) {
        // 接受连接请求
        client_sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // 创建子进程处理连接
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            close(server_sock);
            handle_connection(client_sock);
            close(client_sock);
            exit(EXIT_SUCCESS);
        }
        close(client_sock);
    }

    return 0;
}

void handle_connection(int sock) {
    char buffer[BUFFER_SIZE];
    int n;

    while (1) {
        // 读取命令
        memset(buffer, 0, BUFFER_SIZE);
        n = read(sock, buffer, BUFFER_SIZE);
        if (n == 0) {
            break;
        }
        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // 处理命令
        handle_command(buffer, sock);
    }
}

void handle_command(char *cmd, int sock) {
    char *args[MAX_ARGS];  // 参数缓冲区
    char *cmds[MAX_CMDS];  // 命令缓冲区
    int num_cmds, num_args, cmd_idx, arg_idx;
    int fd[2], in, out;  // 管道和输入/输出流描述符
    pid_t pid;
    char buffer[BUFFER_SIZE];

    // 检查是否退出
    if (strcmp(cmd, "exit\n") == 0) {
        close(sock);
        exit(EXIT_SUCCESS);
    }

    // 初始化参数和命令缓冲区
    num_cmds = 0;
    cmds[num_cmds++] = strtok(cmd, "| \n");
    while ((cmds[num_cmds] = strtok(NULL, "| \n")) != NULL) {
        num_cmds++;
    }

    // 处理每个命令
    in = STDIN_FILENO;  // 初始输入流为标准输入
    for (cmd_idx = 0; cmd_idx < num_cmds; cmd_idx++) {
        num_args = 0;
        args[num_args++] = strtok(cmds[cmd_idx], " \n");
        while ((args[num_args] = strtok(NULL, " \n")) != NULL) {
            num_args++;
        }

        // 创建管道
        if (pipe(fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        out = fd[1];  // 输出流为管道的写端

        // 创建子进程执行命令
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // 重定向输入流
            if (in != STDIN_FILENO) {
                dup2(in, STDIN_FILENO);
                close(in);
            }

            // 重定向输出流
            if (out != STDOUT_FILENO) {
                dup2(out, STDOUT_FILENO);
                close(out);
            }

            // 执行命令
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        // 等待子进程结束
        if (waitpid(pid, NULL, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        // 关闭管道的写端，并将读端作为下一次命令的输入流
        close(fd[1]);
        in = fd[0];
    }

    // 读取命令输出，并发送到客户端
    while ((n = read(in, buffer, BUFFER_SIZE)) > 0) {
        if (write(sock, buffer, n) < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
}
