#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>
#include <sys/stat.h>
#include "functions.c"



//~ #define DFL_PORT 8001
#define BACKLOG 5
#define MSG_LEN 256
#define PACKAGE 60000


//holds message got from client
char buf[MSG_LEN];

char def_dir[MSG_LEN];

//holds currently active dir
char current_dir[PATH_MAX];

//holds received packages
char *temp = NULL;

int listen_fd;
struct sockaddr_in client_addr;
socklen_t client_addr_len = sizeof(struct sockaddr_in);
void listFiles(int client_fd);
void sendFile(int client_fd);
void receiveFile(int client_fd);
long get_package(int socket, long length);
size_t get_line(int socket, long length);
long get_size(char* string);
void changeDir(int client_fd, char* dir);
int checkPath(char* input);
char* constructSize(long length);




void make_server_addr(struct sockaddr_in * addr, unsigned short int port) {
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	addr->sin_port = htons(port);
	memset(&(addr->sin_zero), '\0', 8);

}

void serve(int client_fd) {
	int running = 1;
	size_t readcount;

	while (running && (readcount = get_line(client_fd, MSG_LEN)) > 0) {
		printf("serving\n");

		if (strncmp(buf, "quit", 4) == 0) {
			printf("He doesnt want me any more\n");
			running = 0;
		}		
		if (strncmp(buf, "ls", 2) == 0) {
			listFiles(client_fd);
		}
		if (strncmp(buf, "get ", 4) == 0) {
			sendFile(client_fd);
		}
		if (strncmp(buf, "put ", 4) == 0) {
			receiveFile(client_fd);
		}
		if (strncmp(buf, "cd", 2) == 0) {
			changeDir(client_fd, buf);
		}

		//~ write(client_fd, buf, readcount);

	}

	close(client_fd);

}

int main(int argc, char ** argv) {
	struct sockaddr_in local_address;
	int on = 1;
	int client_fd;
	pid_t pid;
	unsigned int port;

	if (argc == 1) {
		printf("Using default port 8001\n");
		port = 8001;
	} else if (argc == 2) {

		port = strtol(argv[1], NULL, 10);
		if (port == 0) {
			printf("Port address must be positive int\n");
			exit(EXIT_FAILURE);
		}
		printf("Port will be %d\n", port);
	}

	// make a listening socket
	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("main: socket");
		exit(EXIT_FAILURE);
	}
	//make complete server address
	make_server_addr(&local_address, port);
	//bind to siocket to my address
	if (bind(listen_fd, (struct sockaddr *) &local_address,
			sizeof(local_address))) {
		perror("main: bind");
		exit(EXIT_FAILURE);
	}
	//start to listen for incoming connections
	if (listen(listen_fd, BACKLOG)) {
		perror("main: listen");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		perror("main: setsockopt:");
		exit(EXIT_FAILURE);
	}

	printf("Started listening for incoming connections\n");

	getcwd(def_dir, sizeof(def_dir));

	while (1) {
		//oo, somebody is talking to me, i have friends
		client_fd = accept(listen_fd, (struct sockaddr *) &client_addr,
				&client_addr_len);
		if (client_fd < 0) {
			perror("main: accept");
			exit(EXIT_FAILURE);
		}
		printf("Client connected!\n");

		if ((pid = fork()) < 0) {
			perror("server: fork");
			exit(EXIT_FAILURE);
		} else if (!pid) {
			/* child */
			close(listen_fd);
			serve(client_fd);
			printf("Client disconnected!\n");
			exit(EXIT_SUCCESS);

		} else {
			/* parent*/
			close(client_fd);
		}
	}
	close(listen_fd);
	exit(EXIT_SUCCESS);
}

//gets list of current directory items and send it to client
void listFiles(int client_fd) {
	FILE *fp;
	char help[256];
	char *buffer = malloc(0);
	int count = 0;
	char help2[5];
	char *line_p = NULL;
	char *message = malloc(sizeof(char) * 256);

	fp = popen("ls -l", "r");
	if (fp == NULL) {
		perror("Failed to run command\n");
		exit(EXIT_FAILURE);
	}
	while (line_p = fgets(help, sizeof(help), fp)) {
		count = count + strlen(line_p);
		buffer = realloc(buffer, sizeof(char) * count);
		strcat(buffer, line_p);
	}
	pclose(fp);
	strcpy(message, "size:");
	snprintf(help2, 10, "%d", count);
	strcat(message, help2);
	//send length of package
	write(client_fd, message, MSG_LEN);
	//get echo back
	get_line(client_fd, MSG_LEN);
	//compare original and echo
	if (strcmp(message, buf) == 0) {
		write(client_fd, buffer, count);

	}
}

void sendFile(int client_fd) {
	long i = 0;
	int check = 0;
	FILE *input;
	long file_len = 0;
	long location = 0;
	long small_size = 0;
	char *buffer;
	char *fragment;
	char *message = malloc(sizeof(char) * 256);

	//we want files!
	int name_len = strlen(buf) - 4;
	char* filename = malloc(sizeof(char) * name_len);
	strcpy(filename, buf + 4);
	//remove trailing new line if there is one
	if (filename[name_len - 1] == '\n') {
		filename[name_len - 1] = '\0';
	}
	//~ printf("%c", filename[name_len -1]);
	printf("file: %s was wanted\n", filename);
	check = checkPath(filename);
	if (check == 1) {
		printf("It was a file\n");
		//trying to open input file
		input = fopen(filename, "rb");
		if (!input) {
			strcpy(message, "size:-1");
			//send size message to client
			write(client_fd, message, MSG_LEN);
		}else{
			fseek(input, 0, SEEK_END);
			file_len = ftell(input);
			fseek(input, 0, SEEK_SET);
			buffer = (char *) malloc(file_len + 1);
			fread(buffer, file_len, 1, input);
			fclose(input);
			
			//construct and send size message
			message = constructSize(file_len);
			write(client_fd, message, MSG_LEN);
			//get echo back
			get_line(client_fd, MSG_LEN);
			if (strcmp(message, buf) == 0) {
				if (file_len > PACKAGE) {
					for (location = 0; location < file_len;) {
						if (location + PACKAGE < file_len) {
							small_size = PACKAGE;
						} else {
							small_size = file_len - location;
						}
						fragment = malloc(small_size);
						memcpy(fragment, buffer + location, small_size);
						//make size message	
						message = constructSize(small_size);
						//send message to server
						write(client_fd, message, MSG_LEN);
						//get echo back
						i = get_line(client_fd, MSG_LEN);
						if (strcmp(message, buf) == 0) {
							write(client_fd, fragment, small_size);
						}
						location += PACKAGE;
					}
				} else {
					write(client_fd, buffer, file_len);
				}
			}
			
			
		}
		
	}else{
		strcpy(message, "size:-1");
		write(client_fd, message, MSG_LEN);
	}


	

}

void receiveFile(int client_fd) {
	long location = 0, j, file_len;
	long small_size = 0;
	char *message = malloc(sizeof(char) * 256);
	FILE *output;
	unsigned long message_length = 0;
	unsigned long size = 0;
	
	
	int name_len = strlen(buf) - 4;
	char* filename = malloc(sizeof(char) * name_len);
	strcpy(filename, buf + 4);
	//remove trailing new line if there is one
	if (filename[name_len - 1] == '\n') {
		filename[name_len - 1] = '\0';
	}
	//get size package
	message_length = get_line(client_fd, MSG_LEN);
	//get size from message
	file_len = get_size(buf);
	//construct and send response
	message = constructSize(file_len);	
	write(client_fd, message, MSG_LEN);

	output = fopen(filename, "wb");
	fclose(output);
	if (file_len > PACKAGE) {
		output = fopen(filename, "ab");
		for (location = 0; location < file_len;) {
			if (location + PACKAGE < file_len) {
				small_size = PACKAGE;
			} else {
				small_size = file_len - location;
			}

			//get size package
			message_length = get_line(client_fd, MSG_LEN);
			//get size from message
			size = get_size(buf);
			//construct and send response
			message = constructSize(size);			
			write(client_fd, message, MSG_LEN);
			message_length = get_package(client_fd, size);
			
			if (size == message_length) {
				fwrite(temp, small_size, 1, output);
			}else{
				fclose(output);
				output = NULL;
				remove(filename);
				printf("File removed\n");
				return;
			}
			location += PACKAGE;

		}
		printf("File written to server hdd\n");
		if(output){
			fclose(output);
		}		
	} else {
		//get content package
		message_length = get_package(client_fd, file_len);
		if (file_len == message_length) {

			//package size was ok
			output = fopen(filename, "wb");
			fwrite(temp, 1, file_len, output);
			fclose(output);
			printf("File written to server hdd\n");
		} else {
			printf("Problems with package\n");
		}

	}

}
void changeDir(int client_fd, char* directory) {
	char *message = malloc(sizeof(char) * 256);
	int help, size;
	char help2[256];
	if (strlen(directory) == 2) {
		//chdir wont work with this
		strcpy(message, "size:-1");
		//send size message to client
		write(client_fd, message, MSG_LEN);
	} else {
		int name_len = strlen(directory) - 3;
		char* dir = malloc(sizeof(char) * name_len);
		strcpy(dir, directory + 3);
		//remove trailing new line if there is one
		if (dir[name_len - 1] == '\n') {
			dir[name_len - 1] = '\0';
		}
		//now have cd  string and can work magic

		help = chdir(dir);
		printf("answer: %d\n", help);
		if (help != 0) {
			strcpy(message, "size:-1");
			//send size message to client
			write(client_fd, message, MSG_LEN);
		} else {
			getcwd(current_dir, PATH_MAX);
			printf("current dir: %s\n", current_dir);
			size = strlen(current_dir);

			strcpy(message, "size:");
			snprintf(help2, 10, "%d", size);
			strcat(message, help2);
			printf("%s\n", message);
			//send size message to client
			write(client_fd, message, MSG_LEN);
			//get echo back
			get_line(client_fd, MSG_LEN);
			if (strcmp(message, buf) == 0) {
				//~ printf("size was ok\n");
				write(client_fd, current_dir, size);
			}

		}
	}

}
size_t get_line(int socket, long length) {
	size_t count;
	count = read(socket, buf, length);
	if (count < 0) {
		perror("get_line: read");
		exit(EXIT_FAILURE);
	}
	buf[count] = '\0';

	return count;
}


long get_package(int socket, long length) {
	long count = 0;
	temp = malloc(sizeof(char) * length + 1);
	if (!temp) {
		fprintf(stderr, "Memory error!");
		exit(EXIT_FAILURE);
	}
	count = (long) read(socket, temp, length);
	temp[count] = '\0';
	return count;
}
