//checks if given input is file of folder
//1 for file
int checkPath(char* input) {
	struct stat *buff;
	
	buff = malloc(sizeof(struct stat));
	if (!buff) {
		printf("Memory error");
		exit(EXIT_FAILURE);
	}
	stat(input, buff);
	if (buff->st_mode & S_IFREG) {
		return 1;
	} else {
		return 0;
	}

}

//constructs size message from input
char* constructSize(long length) {
	char help2[256];
	char *message;
	
	message = malloc(sizeof(char) * 256);
	if (!message) {
		printf("Memory error");
		exit(EXIT_FAILURE);
	}
	strcpy(message, "size:");
	snprintf(help2, 10, "%lu", length);
	strcat(message, help2);

	return message;
}

char* getFileName(char* input) {
	int name_len = 0;
	char* filename;

	name_len = strlen(input) - 4;
	filename = malloc(sizeof(char) * name_len);
	if (!filename) {
		printf("Memory error");
		exit(EXIT_FAILURE);
	}
	strcpy(filename, input + 4);
	//remove trailing new line if there is one
	if (filename[name_len - 1] == '\n') {
		filename[name_len - 1] = '\0';
	}
	return filename;
}

//get size of package from size message
long get_size(char* string) {
	char *start;
	long size = 0;

	start = strstr(string, ":");
	size = strtol(start + 1, NULL, 10);
	return size;
}
