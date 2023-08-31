// Server
create a socket
bind the socket to an address and port
listen for incoming connections
while true:
    accept a connection
    fork a child process to handle the connection
    in the child process:
        while true:
            receive a command string from the client
            parse the command string to extract commands and arguments
            for each command:
                create a pipe
                fork a child process to execute the command
                in the child process:
                    set up I/O redirection using dup2()
                    execute the command using execvp()
                in the parent process:
                    close the write end of the pipe
                    save the read end of the pipe
            for each saved pipe:
                read the output from the pipe
                send the output to the client using send()
                close the read end of the pipe
            if the last command was "exit":
                break
    close the connection

// Client
create a socket
connect to the server
while true:
    read a command string from the user
    send the command string to the server using send()
    if the command was "exit":
        break
    receive the output from the server using recv()
    display the output to the user
close the socket
