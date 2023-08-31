#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_ARGS 10
#define MAX_COMMANDS 10
#define BUFFER_SIZE 1024
#define MAX_PENDING_CONNECTIONS 5 // added constant

int execute_command(char **args, int in, int out,int sockfd) {
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

/*
int execute_command(char **args, int in, int out) {
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
	return 0;
}
*/
int execute_pipeline(char **commands[], int n,int sockfd) {
	int pipes[MAX_COMMANDS-1][2];
	for (int i = 0; i < n-1; i++) {
		if (pipe(pipes[i]) == -1) {
			perror("pipe");
			return -1;
		}
	}
	int in = STDIN_FILENO;
	for (int i = 0; i < n; i++) {
		int out = sockfd;
		if (i < n-1) {
			out = pipes[i][1];
		}
		if (execute_command(commands[i], in, out, sockfd) == -1) {
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
	//int len=strlen(input);
	char* commands;
	char* temp_commands[MAX_COMMANDS];
	int count=0;
	int j=0;
	int i=0;
	commands=strtok(input,"|");
	while(commands){
		temp_commands[count]=commands;
		count++;
		commands=strtok(NULL,"|");
	}
	for(i=0;i<count;++i){
		j=0;
		commands=strtok(temp_commands[i]," ");
		while (commands)
		{
			args[i][j]=commands;
			//temp_commands[j] = commands;
			j++;
			commands = strtok(NULL, " ");
		}
		args[i][j]=NULL;
	}
	args[count][0]=NULL;
	return count;
}

void handle_client(int sockfd,struct sockaddr_in* client_addr) {
	char buffer[BUFFER_SIZE];
	int n;

	printf("Client connected.\n");

	while (1) {
		bzero(buffer, BUFFER_SIZE); 
		n = read(sockfd, buffer, BUFFER_SIZE);

		if (n <= 0) {
			printf("Client disconnected.\n");
			close(sockfd);
			return;
		}

		buffer[n - 1] = '\0'; // remove newline character
		buffer[n - 2] = '\0'; // remove newline character
		printf("Received from: %s:%d", 
			inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
		printf("\tcommand: %s\n",buffer);
		//printf(strcmp(buffer,"exit"));

		if (strcmp(buffer, "exit") == 0) {
			printf("Client requested to disconnect.\n");
			close(sockfd);
			exit(0);
			return;
		}

		char **commands[MAX_COMMANDS];
		for (int i = 0; i < MAX_COMMANDS; i++) {
			commands[i] = malloc(MAX_ARGS * sizeof(char *));
			for (int j = 0; j < MAX_ARGS; j++) {
				commands[i][j] = NULL;
			}
		}

		//buffer[n-2]="\n";
		int num_commands = parse_input(buffer, commands);

		if (num_commands == 0) {
			printf("No command entered.\n");
			continue;
		}

		if (execute_pipeline(commands, num_commands,sockfd) == -1) {
			printf("Failed to execute command.\n");
		}

		for (int i = 0; i < MAX_COMMANDS; i++) {
			free(commands[i]);
		}
	}
	close(sockfd);
}
int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

 /*
	struct hostent *host;
	cli_addr.sin_family = AF_INET;
	if ((host = gethostbyname(argv[1])) == NULL) {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

	memcpy(&cli_addr.sin_addr.s_addr,host->h_addr_list[0],host->h_length);
	cli_addr.sin_port= htons(portno);
*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}

	listen(sockfd, MAX_PENDING_CONNECTIONS);

	clilen = sizeof(cli_addr);

	pid_t wpid;
	int status=0;
	while (1) {
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr,
				&clilen);

		if (newsockfd < 0) {
			perror("ERROR on accept");
			exit(1);
		}

		//connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(cli_addr));

        pid_t pid = fork();
        if (pid == 0) {
            // Child process - handle client
            handle_client(newsockfd,&cli_addr);
            //exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else {
			//while ((wpid = wait(&status)) > 0);
			close(newsockfd);
			continue;
		}
	}

	while ((wpid = wait(&status)) > 0);
	close(sockfd);
	return 0;
}