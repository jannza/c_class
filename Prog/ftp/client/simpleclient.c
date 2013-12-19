#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include "functions.c"

#define DFL_PORT 8001
#define MSG_LEN 256
#define PACKAGE 60000

int ReadLine(char *);
size_t get_line(int socket, long length);
long get_package(int socket, long length);
long get_size(char* string);
void listFiles(int socket, char* command);
void getFile(int socket, char* filename);
void sendFile(int socket, char* filename);
char* changeDr(int socket, char* dir, char* current);
char* getFileName(char* input);
char* constructSize(long length);
int checkPath(char* input);

//holds received messages
char buf[MSG_LEN];
//holds received packages
char *temp = NULL;

int main(int argc, char *argv[]) {
	//holds current directory data
	char* current_dir = malloc(PATH_MAX);
	//just in case directory needs backup
	char* alternate_dir = malloc(PATH_MAX);
	int on = 1;
	struct sockaddr_in server; /* Server's address assembled here */
	struct hostent *host_info;
	int sock, i;
	char line[MSG_LEN]; /* Buffer to copy from user to server */
	char *server_name;
	unsigned int port;
	/* Get server name from parameter. If none, use localhost */

	if (argc == 1) {
		printf("Using default port 8001 and localhost\n");
		port = DFL_PORT;
		server_name = "localhost";
	} else if (argc == 2) {

		port = strtol(argv[1], NULL, 10);
		if (port == 0) {
			server_name = argv[1];
			port = DFL_PORT;
		} else {
			server_name = "localhost";
		}

	} else if (argc == 3) {
		server_name = argv[1];
		port = strtol(argv[2], NULL, 10);
		if (port == 0) {
			printf("Port address must be positive int\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Create the socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("creating stream socket");
		exit(EXIT_FAILURE);
	}

	host_info = gethostbyname(server_name);
	if (host_info == NULL) {
		fprintf(stderr, "%s: unknown host: %s\n", argv[0], server_name);
		exit(EXIT_FAILURE);
	}

	/* Set up the server's socket address, then connect */
	server.sin_family = host_info->h_addrtype;
	memcpy((char *) &server.sin_addr, host_info->h_addr, host_info->h_length);
	server.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *) &server, sizeof server) < 0) {
		perror("connecting to server");
		exit(EXIT_FAILURE);
	}

	/* We are connected to the server.
	 Read a line from the user, and write it to the socket.
	 Read the return value from the server, and print it to the
	 screen/standard output.
	 */

	printf("Connected to server %s\n", server_name);
	current_dir = changeDr(sock, "cd .", current_dir);

	while (1) {
		printf("%s> ", current_dir);
		i = ReadLine(line);

		//quitting program
		if (strncmp(line, "quit", 4) == 0) {
			write(sock, line, MSG_LEN);
			break;
		}
		//trying to work ls command
		if (strncmp(line, "ls", 2) == 0) {
			listFiles(sock, line);

		}
		//getting a file from server
		if (strncmp(line, "get ", 4) == 0) {
			char* filename = getFileName(line);
			write(sock, line, MSG_LEN);
			getFile(sock, filename);

		}
		//sending a file to server
		if (strncmp(line, "put ", 4) == 0) {
			char* filename = getFileName(line);
			sendFile(sock, filename);

		}
		//changing directory
		if (strncmp(line, "cd", 2) == 0) {
			current_dir = changeDr(sock, line, current_dir);

		}

	}

	close(sock);

	return 0;
}
void listFiles(int socket, char* command) {
	int i;
	int size = 0;
	//hold constructed messages to server
	char *message = malloc(sizeof(char) * 256);
	
	//take only first 2 letters from ls command, else not needed
	strncpy(message, command, 2);
	//send initial message
	write(socket, message, MSG_LEN);
	//get size of package
	i = get_line(socket, MSG_LEN);
	//get size from message
	size = get_size(buf);
	//construct and send
	message = constructSize(size);
	write(socket, message, MSG_LEN);

	//get size from message
	size = get_size(buf);
	//get content package
	i = get_package(socket, size);
	//compare
	if (size == strlen(temp)) {
		//package size was ok
		printf("%s", temp);
	} else {
		printf("Problems with package\n");
	}
}

void getFile(int socket, char* filename) {
	long location = 0;
	long small_size = 0;
	long  file_len = 0;
	long size = 0;
	FILE *file;
	char *message = malloc(sizeof(char) * 256);
	long message_length = 0;

	//get size package
	get_line(socket, MSG_LEN);
	//get preliminary size from message
	file_len = get_size(buf);
	if (file_len == -1) {
		printf("File doesnt exist\n");
	} else {
		message = constructSize(file_len);
		write(socket, message, MSG_LEN);
		
		file = fopen(filename, "wb");
		fclose(file);
		if (file_len > PACKAGE) {
			file = fopen(filename, "ab");
			for (location = 0; location < file_len;) {
				if (location + PACKAGE < file_len) {
					small_size = PACKAGE;
				} else {
					small_size = file_len - location;
				}
				//get size package
				message_length = get_line(socket, MSG_LEN);
				//get size from message
				size = get_size(buf);
				//construct and send response
				message = constructSize(size);			
				write(socket, message, MSG_LEN);
				
				
				message_length = get_package(socket, size);
				if (size == message_length) {
					fwrite(temp, small_size, 1, file);

				}else{
					fclose(file);
					file = NULL;
					remove(filename);
					printf("File removed\n");
					break;
				}
				location += PACKAGE;

			}
			if(file){
				fclose(file);
			}	
			
		}else{
			//get content package
			message_length = get_package(socket, file_len);
			if (file_len == message_length) {

				//package size was ok
				file = fopen(filename, "wb");
				fwrite(temp, 1, file_len, file);
				fclose(file);
			} else {
				printf("Problems with package\n");
			}
		}
		
		
		
	}

}

void sendFile(int socket, char* filename) {
	int check = 0;
	long location = 0;
	long small_size = 0;
	long i = 0;
	long file_len = 0;
	FILE *input;
	char *buffer;
	char *fragment;
	char *message = malloc(sizeof(char) * 256);
	if (!message) {
		printf("malloc error\n");
	}
	check = checkPath(filename);
	if (check == 1) {
		//input really is a file
		//trying to open input file
		input = fopen(filename, "rb");
		if (!input) {
			printf("No such file in client directory.\n");
		} else {
			fseek(input, 0, SEEK_END);
			file_len = ftell(input);
			fseek(input, 0, SEEK_SET);
			buffer = (char *) malloc(file_len + 1);
			fread(buffer, file_len, 1, input);
			fclose(input);

			strcpy(message, "put ");
			strcat(message, filename);
			//send initial request
			write(socket, message, MSG_LEN);
			//make size message
			message = constructSize(file_len);
			//send message to server
			write(socket, message, MSG_LEN);
			//get echo back
			i = get_line(socket, MSG_LEN);
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
						write(socket, message, MSG_LEN);
						//get echo back
						i = get_line(socket, MSG_LEN);
						if (strcmp(message, buf) == 0) {
							write(socket, fragment, small_size);
						}
						location += PACKAGE;
					}
				} else {
					write(socket, buffer, file_len);
				}

			}

		}

	} else {
		printf("I can only send files, nothing else.\n");
	}

}

char* changeDr(int socket, char* dir, char* current) {
	int size = 0;
	char *helper;
	char *message;
	
	helper = malloc(PATH_MAX);
	message = malloc(sizeof(char) * 256);
	strcpy(helper, current);

	//send initial message
	write(socket, dir, MSG_LEN);
	//get size package
	get_line(socket, MSG_LEN);
	//get preliminary size from message
	size = get_size(buf);
	if (size == -1) {
		printf("Cant change to specified directory\n");
		return helper;
	} else {
		//construct and send
		message = constructSize(size);
		write(socket, message, MSG_LEN);
		//get content package
		size = get_package(socket, size);
		return temp;
	}

}

int ReadLine(char *line) {
	int i, c;
	for (i = 0; i < MSG_LEN - 1 && (c = getchar()) != EOF && c != '\n'; i++)
		line[i] = c;
	line[i] = '\0';
	return i;
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
	long count;
	
	temp = malloc(sizeof(char) * length);
	if (!temp) {
		fprintf(stderr, "Memory error!");
		exit(EXIT_FAILURE);
	}
	count = (long) read(socket, temp, length);
	temp[count] = '\0';
	return count;
}
