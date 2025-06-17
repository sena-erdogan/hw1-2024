#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define WRITE_PERMS (S_IRUSR | S_IWUSR | S_IWGRP)

#define LOG_FILE "log.txt"
#define MAX_LENGTH 128
#define MAX_NUM 1024

pid_t childPid;
char grades[][3] = {"AA", "BA", "BB", "CB", "CC", "DC", "DD", "FF", "NA", "VF"};

void signal_handler(int signal) {
	if(signal == SIGINT) {
		printf("\nReceived SIGINT\n\n");
		
		printf("%d ", childPid);
		
		kill(childPid, SIGKILL);
		
        	exit(EXIT_FAILURE);
	}
}

char* get_timestamp(){
	time_t now = time(NULL);
	return asctime(localtime(&now));
}

void write_to_log(char* log, int success){
	int logFd;
	
	logFd = open(LOG_FILE, WRITE_FLAGS , WRITE_PERMS);
	
	if(logFd == -1)	printf("Could not open log file: %s", LOG_FILE);
	
	char timestamp[25];
	strcpy(timestamp, strtok(get_timestamp(), "\n"));
	
   	write(logFd, timestamp, strlen(timestamp));
   	write(logFd, " ", 1);
   	if(success==0)	write(logFd, "UNSUCCESSFUL", strlen("UNSUCCESSFUL"));
   	else if(success==1)	write(logFd, "SUCCESSFUL", strlen("SUCCESSFUL"));
   	write(logFd, "\t", 1);
   	write(logFd, log, strlen(log));
   	write(logFd, "\n", 1);
	
	if(close(logFd) == -1)	printf("Could not close log file: %s", LOG_FILE);
}

int gtuStudentGrades(const char* file){
	int fd;
	
	fd = open(file, WRITE_FLAGS | O_TRUNC , WRITE_PERMS);
	
	if(fd == -1){
		printf("Could not open file: %s", file);
		return 0;
	}
	
	if(close(fd) == -1){
		printf("Could not close file: %s", file);
		return 0;
	}
	
	return 1;
}

int addStudentGrade(const char* file, const char* name, const char* grade){
	int fd;
	
	fd = open(file, WRITE_FLAGS , WRITE_PERMS);
	
	if(fd == -1){
		printf("Could not open file: %s", file);
		return 0;
	}
	
	write(fd, name, strlen(name));
   	write(fd, ", ", 1);
   	write(fd, grade, strlen(grade));
   	write(fd, "\n", 1);
	
	if(close(fd) == -1){
		printf("Could not close file: %s", file);
		return 0;
	}
	
	return 1;
}

int searchStudent(const char* file, const char* name){
	int fd;
	int bytesread = 0;
	int i=0;
	char buf[1];
	
	fd = open(file, READ_FLAGS);
	
	if(fd == -1){
		printf("Could not open file: %s", file);
		return 0;
	}
	
	while(((bytesread = read(fd, buf, 1)) > 0)){
		if(buf[0] == name[i] && i < strlen(name)){
			i++;
		}else if(i == strlen(name)){
			bytesread = read(fd, buf, 1);
			
			if(bytesread <= 0)	break;
				
			printf("Grade of %s: %c", name, buf[0]);
				
			bytesread = read(fd, buf, 1);
			
			if(bytesread <= 0)	break;
				
			printf("%c\n\n", buf[0]);
			
			break;
		}else{
			i=0;
			while(buf[0] != '\n'){
				bytesread = read(fd, buf, 1);
			
				if(bytesread <= 0)	break;
			}
		}	
	}
	
	if(close(fd) == -1){
		printf("Could not close file: %s", file);
		return 0;
	}
	
	return 1;
}

int showAll(const char* file){
	int fd;
	int bytesread = 0;
	char buf[1];
	
	fd = open(file, READ_FLAGS);
	
	if(fd == -1){
		printf("Could not open file: %s", file);
		return 0;
	}
	
	while(((bytesread = read(fd, buf, 1)) > 0)){
			
		printf("%c", buf[0]);
	}
	
	if(close(fd) == -1){
		printf("Could not close file: %s", file);
		return 0;
	}
	
	return 1;
}

int listGrades(const char* file){
	int fd;
	int bytesread = 0;
	int line=0;
	char buf[1];
	
	fd = open(file, READ_FLAGS);
	
	if(fd == -1){
		printf("Could not open file: %s", file);
		return 0;
	}
	
	while(((bytesread = read(fd, buf, 1)) > 0)){
		
		if(line == 5)	break;
		
		if(buf[0] == '\n')	line++;
			
		printf("%c", buf[0]);
	}
	
	if(close(fd) == -1){
		printf("Could not close file: %s", file);
		return 0;
	}
	
	return 1;
} 

int listSome(const char* file, int numofEntries, int pageNumber){
	int fd;
	int bytesread = 0;
	int line=0;
	char buf[1];
	
	fd = open(file, READ_FLAGS, 0666);
	
	if(fd == -1){
		printf("Could not open file: %s", file);
		return 0;
	}
	
	while((bytesread = read(fd, buf, 1)) > 0){
		if(buf[0] == '\n'){
			if(line >= (numofEntries * (pageNumber-1))){
				write(STDOUT_FILENO, buf, 1);
			}
			line +=1;	
		}
		
		else if(line >= (numofEntries * (pageNumber-1))){
			write(STDOUT_FILENO, buf, 1);
		}
		
		if(line == (numofEntries * pageNumber)){
			break;
		}
	}		
	
	if(close(fd) == -1){
		printf("Could not close file: %s", file);
		return 0;
	}
	
	return 1;
}

int compare(const void* first, const void* second) {
    return strcmp(first, second);
}

int sortAll(const char* file){
	int fd;
	int bytesread = 0;
	int i=0;
	int j=0;
	char buf[1];
	char line[MAX_NUM][MAX_LENGTH];
	
	fd = open(file, READ_FLAGS);
	
	if(fd == -1){
		printf("Could not open file: %s", file);
		return 0;
	}
	
	while(((bytesread = read(fd, buf, 1)) > 0) && i<=MAX_NUM){
		if(buf[0] == '\n'){
			line[i][j] = '\0';
			i++;
			j=0;
		}else{
			line[i][j] = buf[0];
			j++;
		}
	}
	
	qsort(line, i, MAX_LENGTH, compare);
	
	for (j = 0; j < i; j++) {
		printf("%s\n", line[j]);
	}
	
	if(close(fd) == -1){
		printf("Could not close file: %s", file);
		return 0;
	}
	
	return 1;
}

int main(){

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &signal_handler;
	sigaction(SIGINT, &sa, NULL);
	
	char input[MAX_LENGTH];
	char command[MAX_LENGTH];
	char file[MAX_LENGTH];
	char name[MAX_LENGTH];
	char grade[5];
	char fullCommand[MAX_LENGTH];
	char numofEntries[5];
	char pageNumber[5];
	int pid;
	int flag=0;

	while(1){
		memset(input, '\0', sizeof(input));
		memset(command, '\0', sizeof(command));
	
		read(STDIN_FILENO, input, sizeof(input));
		input[strlen(input)-1] = '\0';
		
		if(input[0] == '\n'){	continue;
		}
		
		printf("%s\n", command);
		printf("%s\n", input);
		strcpy(fullCommand, input);
		strcpy(command, strtok(input, " "));
		
		pid = fork();
		
		if(pid == -1){
			perror("Could not create child process");
			exit(1);
		}else if (pid == 0){
			childPid = getpid();
			if(strcmp(command, "gtuStudentGrades") == 0 &&  strlen(fullCommand)==16){
				printf("Available commands are as follows\n\n");
				printf("gtuStudentGrades \"grades.txt\"\n");
				printf("addStudentGrade \"grades.txt\" \"Name Surname\" \"AA\"\n");
				printf("searchStudent \"grades.txt\" \"Name Surname\"\n");
				printf("sortAll \"gradest.txt\"\n");
				printf("showAll \"grades.txt\"\n");
				printf("listGrades \"grades.txt\"\n");
				printf("listSome \"numofEntries\" \"pageNumber\" \"grades.txt\"\n\n");
				
				write_to_log(input, 1);
			}else if(strcmp(command, "gtuStudentGrades") == 0){
				strcpy(file, strtok(NULL, " "));
				char *start = strchr(file, '"');
				if(start != NULL){
				    char *end = strchr(start + 1, '"');
				    if(end != NULL){
					strncpy(file, start + 1, end - start - 1);
					file[end - start - 1] = '\0';
					write_to_log(fullCommand, gtuStudentGrades(file));
				    }else{
				    	write_to_log(fullCommand, 0);
					printf("\nMissing closing quotation mark\n");
				    }
				}else{
				    write_to_log(fullCommand, 0);
				    printf("\nMissing opening quotation mark\n");
				}
				
			}else if(strcmp(command, "addStudentGrade") == 0){
				strcpy(file, strtok(NULL, " "));
				strcpy(name, strtok(NULL, " "));
				strcat(name, " ");
				strcat(name, strtok(NULL, " \""));
				strcpy(grade, strtok(NULL, " "));
				
				char *start_file = strchr(fullCommand, '"');
				
				if(start_file != NULL){
					char *end_file = strchr(start_file + 1, '"');
					if(end_file != NULL){
						strncpy(file, start_file + 1, end_file - start_file - 1);
						file[end_file - start_file - 1] = '\0';

						char *start_name = strchr(end_file + 1, '"');
						if(start_name != NULL){
							char *end_name = strchr(start_name + 1, '"');
							if(end_name != NULL){
								strncpy(name, start_name + 1, end_name - start_name - 1);
								name[end_name - start_name - 1] = '\0';
								
								char *start_grade = strchr(end_name + 1, '"');
								if(start_grade != NULL){
									char *end_grade = strchr(start_grade + 1, '"');
									if(end_grade != NULL){
										strncpy(grade, start_grade + 1, end_grade - start_grade - 1);
										grade[end_grade - start_grade - 1] = '\0';
										
										for (int i = 0; i < sizeof(grades) / sizeof(grades[0]); i++) {
											if(strcmp(grades[i], grade) == 0) {
												write_to_log(fullCommand, addStudentGrade(file, name, grade));
												flag=1;
											}
										}
										if(flag==0){
											write_to_log(fullCommand, 0);
											printf("\nInvalid grade\n");
										}
									}else{
										write_to_log(fullCommand, 0);
										printf("\nMissing closing quotation mark for grade\n");
									}
								}else{
									write_to_log(fullCommand, 0);
									printf("\nMissing opening quotation mark for grade\n");
								}
							}else{
								write_to_log(fullCommand, 0);
								printf("\nMissing closing quotation mark for name\n");
							}
						}else{
							write_to_log(fullCommand, 0);
							printf("\nMissing opening quotation mark for name\n");
						}
					}else{
					write_to_log(fullCommand, 0);
					printf("\nMissing closing quotation mark for file\n");
					}
				}else{
					write_to_log(fullCommand, 0);
					printf("\nMissing opening quotation mark for file\n");
				}
			}else if(strcmp(command, "searchStudent") == 0){
				strcpy(file, strtok(NULL, " "));
				strcpy(name, strtok(NULL, " "));
				strcat(name, strtok(NULL, " "));
				
				char *start_file = strchr(fullCommand, '"');
				
				if(start_file != NULL){
					char *end_file = strchr(start_file + 1, '"');
					if(end_file != NULL){
						strncpy(file, start_file + 1, end_file - start_file - 1);
						file[end_file - start_file - 1] = '\0';

						char *start_name = strchr(end_file + 1, '"');
						if(start_name != NULL){
							char *end_name = strchr(start_name + 1, '"');
							if(end_name != NULL){
								strncpy(name, start_name + 1, end_name - start_name - 1);
								name[end_name - start_name - 1] = '\0';
								
								write_to_log(fullCommand, searchStudent(file, name));
							}else{
								write_to_log(fullCommand, 0);
								printf("\nMissing closing quotation mark for name\n");
							}
						}else{
							write_to_log(fullCommand, 0);
							printf("\nMissing opening quotation mark for name\n");
						}
					}else{
					write_to_log(fullCommand, 0);
					printf("\nMissing closing quotation mark for file\n");
					}
				}else{
					write_to_log(fullCommand, 0);
					printf("\nMissing opening quotation mark for file\n");
				}
			}else if(strcmp(command, "showAll") == 0){
				strcpy(file, strtok(NULL, " "));
				char *start = strchr(file, '"');
				if(start != NULL){
				    char *end = strchr(start + 1, '"');
				    if(end != NULL){
					strncpy(file, start + 1, end - start - 1);
					file[end - start - 1] = '\0';
					write_to_log(fullCommand, showAll(file));
				    }else{
				    	write_to_log(fullCommand, 0);
					printf("\nMissing closing quotation mark\n");
				    }
				}else{
				    write_to_log(fullCommand, 0);
				    printf("\nMissing opening quotation mark\n");
				}
				
			}else if(strcmp(command, "listGrades") == 0){
				strcpy(file, strtok(NULL, " "));
				char *start = strchr(file, '"');
				if(start != NULL){
				    char *end = strchr(start + 1, '"');
				    if(end != NULL){
					strncpy(file, start + 1, end - start - 1);
					file[end - start - 1] = '\0';
					write_to_log(fullCommand, listGrades(file));
				    }else{
				    	write_to_log(fullCommand, 0);
					printf("\nMissing closing quotation mark\n");
				    }
				}else{
				    write_to_log(fullCommand, 0);
				    printf("\nMissing opening quotation mark\n");
				}
				
			}else if(strcmp(command, "listSome") == 0){
				strcpy(numofEntries, strtok(NULL, " "));
				strcpy(pageNumber, strtok(NULL, " "));
				strcpy(file, strtok(NULL, " "));
				
				char *start_file = strchr(fullCommand, '"');
				
				if(start_file != NULL){
					char *end_file = strchr(start_file + 1, '"');
					if(end_file != NULL){
						strncpy(numofEntries, start_file + 1, end_file - start_file - 1);
						numofEntries[end_file - start_file - 1] = '\0';

						char *start_name = strchr(end_file + 1, '"');
						if(start_name != NULL){
							char *end_name = strchr(start_name + 1, '"');
							if(end_name != NULL){
								strncpy(pageNumber, start_name + 1, end_name - start_name - 1);
								pageNumber[end_name - start_name - 1] = '\0';
								
								char *start_grade = strchr(end_name + 1, '"');
								if(start_grade != NULL){
									char *end_grade = strchr(start_grade + 1, '"');
									if(end_grade != NULL){
										strncpy(file, start_grade + 1, end_grade - start_grade - 1);
										file[end_grade - start_grade - 1] = '\0';
										
										write_to_log(fullCommand, listSome(file, atoi(numofEntries), atoi(pageNumber)));
									}else{
										write_to_log(fullCommand, 0);
										printf("\nMissing closing quotation mark for file\n");
									}
								}else{
									write_to_log(fullCommand, 0);
									printf("\nMissing opening quotation mark for file\n");
								}
							}else{
								write_to_log(fullCommand, 0);
								printf("\nMissing closing quotation mark for pageNumber\n");
							}
						}else{
							write_to_log(fullCommand, 0);
							printf("\nMissing opening quotation mark for pageNumber\n");
						}
					}else{
					write_to_log(fullCommand, 0);
					printf("\nMissing closing quotation mark for numofEntries\n");
					}
				}else{
					write_to_log(fullCommand, 0);
					printf("\nMissing opening quotation mark for numofEntries\n");
				}
			}else if(strcmp(command, "sortAll") == 0){
				strcpy(file, strtok(NULL, " "));
				char *start = strchr(file, '"');
				if(start != NULL){
				    char *end = strchr(start + 1, '"');
				    if(end != NULL){
					strncpy(file, start + 1, end - start - 1);
					file[end - start - 1] = '\0';
					write_to_log(fullCommand, sortAll(file));
				    }else{
				    	write_to_log(fullCommand, 0);
					printf("\nMissing closing quotation mark\n");
				    }
				}else{
				    write_to_log(fullCommand, 0);
				    printf("\nMissing opening quotation mark\n");
				}
				
			}else{
				printf("\nInvalid commmand. Please try one of the valid ones :)\n\n");
				printf("gtuStudentGrades \"grades.txt\"\n");
				printf("addStudentGrade \"grades.txt\" \"Name Surname\" \"AA\"\n");
				printf("searchStudent \"grades.txt\" \"Name Surname\"\n");
				printf("sortAll \"gradest.txt\"\n");
				printf("showAll \"grades.txt\"\n");
				printf("listGrades \"grades.txt\"\n");
				printf("listSome \"numofEntries\" \"pageNumber\" \"grades.txt\"\n\n");
			}
		}else{
			int status;
			waitpid(childPid, &status, 0);
		}
	}
	
	return 0;
}
