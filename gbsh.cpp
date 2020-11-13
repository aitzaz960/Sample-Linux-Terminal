#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<limits.h>
#include<sys/wait.h>
#include<dirent.h>
#include<cstring>

extern char **environ;
char** command;
int input_array_size=100;
int no_of_arguments=0;
int status;
bool output_redirection=false;
int output_redirection_index=0;
int total_commands=0;
int file_desc_out, last_screen_out;

void Tokenize()
{
	printf("no of arguments=%d\n", no_of_arguments);			//making token with respect to space
	for(int i=0; i<no_of_arguments; i++)
		printf("%s\n", command[i]);
}

int len(char* command)								//this will calculate length of char array
{
	int size=0;
	for(int index=0; (command[index] != '\n' && command[index] != '\0'); index++)
		size++;
	return size;
}

void seperate_arguments(char* arr)			//this function will make tokens, incase of input redirection will make token of that file's data and in output redirection case will store the name of file and valueable info
{
	int temp_size=0, index=0, begin_point=0;				//will be used in writing in the tokens
	bool to_write=false;							//means there is some data which needs to be written as a token
	bool file_found=false;							//means that I/O redirection has found
	char read_data[1024];							//input file can contain any amount of data so keeping its ize big
	for(int i=0; (arr[i] != '\n' && arr[i] != '\0'); i++)			//command will be terminated by either enter or \0
	{	
		while (arr[i] == ' ')						//we dont have any use of space
			i++;							//ignoring initial whitespaces which may be typed unintentionally
		if (arr[i] == '\n' || arr[i] == '\0')
			break;							//in that unentionalcase the user would not have enetered any command
		begin_point=i;
		while (arr[i] != ' ' && arr[i] != '\n' && arr[i] != '\0')	//this will run for each token and will tell us its size		
		{
			if (arr[i] == '<' && arr[i+1]==' ' && (arr[i+2]!=' ' && arr[i+2]!='\n' && arr[i+2]!='\0') )		//incase of > make file_found true
			{
				i += 2;
				begin_point=i;
				file_found=true;
			}
			if (arr[i] == '>' && arr[i+1]==' ' && (arr[i+2]!=' ' && arr[i+2]!='\n' && arr[i+2]!='\0') )		//changing the value of bool
			{
				i += 2;
				begin_point=i;
				output_redirection=true;
				output_redirection_index=index;
			}
			to_write=true;						//if loop can reach here means there is something to make token of and thus to_write=true
			temp_size++;						//this will hole size of this specific token
			i++;
		}
		if (to_write == true)
		{
			command[index] = (char*) malloc(temp_size+1);		//alocating space for token
			for(int j=0; j<temp_size; j++)				//copying data
				command[index][j]=arr[begin_point++];
			command[index][temp_size]='\0';				//at the end place null character to avoid garbage
			if (file_found == true)					//if there was a inputfile now its time to handle it
			{
				int file_desc = open(command[index], O_RDONLY);		//opening specific file
				if (file_desc<0)
				{
					printf("%s couldnot be opened\n", command[index]);
				}
				else
				{
					int last_screen = dup(0);		//changing input descriptors so we can read from the file
					dup2(file_desc, 0);
					scanf("%s", read_data);
					dup2(last_screen, 0);
					free(command[index]);
					command[index] = (char*) malloc( len(read_data)+1 );
					for(int j=0; j<len(read_data); j++)
						command[index][j]=read_data[j];
					command[index][ len(read_data) ]= '\0';	 //now we have repalced the name of file with the info on that file 						
				}
				file_found=false;		//as we have copied data so now make it false to reuse it
			}
			index++;
			if (output_redirection == false)			//outputfile is just a printing place not any part of the command
				no_of_arguments++;
		}
		begin_point=0;
		temp_size=0;
		to_write=false;
	}
}

void destroy_space(char** input)
{
	for(int i=0; i<15; i++)				//destroying the dynamic memory
		free(input[i]);
	free(input);
	no_of_arguments=0;
}


int no_of_commands(char* arr)				//this will tell no of commands which can be utilized in piping
{
	int number=0;
	bool alpha=false;
	for(int i=0; (arr[i] != '\n' && arr[i] != '\0'); i++)
	{
		if ( (arr[i] >= 'A' && arr[i] <='Z') || (arr[i] >='a' && arr[i] <= 'z') || (arr[i] >= '0' && arr[i] <= '9') )		//means there is some valid data
		{
			alpha=true;
		}
		if (arr[i]=='\n' || arr[i]=='\0' || arr[i]=='|')				//here | along with other terminating characters means end of 1 command
		{	
			if (alpha==true)
			{
				number++;							//increamenting in no of commands
				alpha=false;
			}
			if (arr[i] == '\n' || arr[i]=='\0')
			{
				break;	
			}
		}
		if (arr[i] == '|' && arr[i+1]==' ')
		{
			if (alpha)
			{
				number++;
				alpha=false;	
			}
		}
	}
	if (alpha == true)
	{
		alpha=false;
		number++;
	}
	return number;
}

int main(int argc, char *argv[]) 
{
//	int name_length=1024;
	char username[name_length], host_name[name_length], cwd[name_length];
	char arr[100];
	command = (char**) malloc(15);							//this will be used to stote tokens seperated by space
	getlogin_r(username, name_length);						//for showing the line of terminal and its detail 
	gethostname(host_name, name_length);
	getcwd(cwd, name_length);
	while (1)									//the terminal will keep on running
	{
		memset(&arr, 0, sizeof(arr));						//after every command array is cleaned so it may not repeat previous command
		no_of_arguments=0;							//each time after the command is run all its data should be lost
		printf("%s@%s %s$ ", username, host_name, cwd);	
		for(int i=0; i<input_array_size; i++)					//taking input the command character by character until enter key is pressed
		{
			scanf("%c", arr+i);
			if (arr[i] == '\n')
				break;
		}
		if (arr[0] == '\n')							//in case of empty command keep on going
			continue;
		seperate_arguments(arr);					//this will make tokens replace input files with their data and hold redirection info
		if (output_redirection)						//if we have output redirection than change the descriptor
		{	
			file_desc_out = open(command[output_redirection_index], O_WRONLY | O_CREAT | O_TRUNC, 0777);
			last_screen_out = dup(1);
			dup2(file_desc_out, 1);
		}
		if (command[0][0]=='e' && command[0][1]=='x' && command[0][2]=='i' && command[0][3]=='t' && command[0][4]=='\0')
		{
			exit(0);							//exit command will simply exit the shell
		}
		else if (command[0][0]=='p' && command[0][1]=='w' && command[0][2]=='d' && command[0][3]=='\0')	
		{	
			printf("%s\n", getcwd(cwd, name_length));			//pwd will print the present working directory
		}
		else if (command[0][0]=='c' && command[0][1]=='l' && command[0][2]=='e' && command[0][3]=='a' && command[0][4]=='r' && command[0][5]=='\0')
		{
			system("clear");						//clear command will clear the screen
		}
		else if (command[0][0]=='l' && command[0][1]=='s')			//ls can be of two types 1-> with directory name 2-> without directory name
		{
			struct dirent *de;						//structure for holding name of directories/files
			DIR *dr;
			if (no_of_arguments==1)						//if without any argument
			{				
				dr= opendir(".");					//then we will consider the current directory represented by .
				if (dr == NULL)
				{
					printf("Couldn't open current directory\n");
				}
				else
				{
					while (de = readdir(dr))
					{
						printf("%s\t", de->d_name);
					}
					printf("\n");
				}
				closedir(dr);
				 
			}
			else if (no_of_arguments == 2)					//if with theargument
			{
				dr= opendir(command[1]);				//then we will explore that directory its name will be placed as 2nd token 
				if (dr == NULL)
				{
					printf("Couldn't open current directory\n");
				}
				else
				{
					while (de = readdir(dr))
					{
						printf("%s\t", de->d_name);
					}
					printf("\n");
				}
				closedir(dr);
			}
			else
				Tokenize();					//if more than this atguments specified means unknown command and we just print it
		}
		else if (command[0][0]=='c' && command[0][1]=='d' && command[0][2]=='\0')		//cd can also be of same two types
		{
			if (no_of_arguments==1)					//if no additional argumet specified then we will consider it as home
			{
				if (chdir("/home") != 0)
					perror("chdir() to /home failed");
				getcwd(cwd, name_length);
			}
			else if (no_of_arguments == 2)				//else change to that directory
			{
				if (chdir(command[1]) != 0)
					perror("chdir() failed");
				getcwd(cwd, name_length);
			}	
			else
			{
				printf("Too much arguments specified for cd command\n");
			}
		}
		else if (command[0][0]=='e' && command[0][1]=='n' && command[0][2]=='v' && command[0][3]=='i' && command[0][4]=='r' && command[0][5]=='o' && command[0][6]=='n' && command[0][7]=='\0')
		{
									//if environ is typed then we will print all environment variables
			for(int i=0; environ[i] != NULL; i++)
				printf("%s\n", environ[i]);
			printf("\n\n");
		}
		else if (command[0][0]=='s' && command[0][1]=='e' && command[0][2]=='t' && command[0][3]=='e' && command[0][4]=='n' && command[0][5]=='v' && command[0][6]=='\0')
		{
			if (no_of_arguments==2)				//in case of <setenv> <variable> we will set variable to empty string
			{
				setenv(command[1], "", 1);
			}
			else if (no_of_arguments==3) 
			{
				setenv(command[1], command[2], 1);
			}
		}
		else if (command[0][0]=='u' && command[0][1]=='n' && command[0][2]=='s' && command[0][3]=='e' && command[0][4]=='t' && command[0][5]=='e' && command[0][6]=='n' && command[0][7]=='v' && command[0][8]=='\0')
		{
			if (no_of_arguments==2)				//deleting the environment variable 
			{
				unsetenv(command[2]);
			}
			else 
				Tokenize();
		}
		else if (command[0][0]=='t' && command[0][1]=='o' && command[0][2]=='p' && command[0][3]=='\0')
		{
			pid_t id1 = fork();
			if (id1 == 0)
			{
				execlp("top", "top", NULL);			//we just had to use the top command
			}
			else if (id1 > 0)
			{
				wait(&status);
			}
		}
		else if (command[0][0]=='p' && command[0][1]=='s' && command[0][2]=='\0')
		{
			pid_t id1 = fork();
			if (id1 == 0)
			{
				execlp("ps", "ps", NULL);
			}
			else if (id1 > 0)
			{
				wait(&status);
			}
		}
		else if (command[0][0]=='m' && command[0][1]=='a' && command[0][2]=='n' && command[0][3]=='\0')
		{
			if (no_of_arguments == 1)			//there must be a command which we should display manual of
			{
				printf("Too few arguments mentioned for man command!\n");
			}
			else
			{
				pid_t id1 = fork();
				if (id1 == 0)
				{
					execlp("man", "man", command[1], NULL);
				}
				else if (id1 > 0)
				{
					wait(&status);
				}
			}
		}
		else if (command[0][0]=='c' && command[0][1]=='s' && command[0][2]=='h' && command[0][3]=='\0')
		{
			pid_t id1 = fork();
			if (id1 == 0)
			{
				execlp("csh", "csh", NULL);
			}
			else if (id1 > 0)
			{
				wait(&status);
			}
		}
		else
			Tokenize();
		if (output_redirection)			//if there was output redirection we should change the descriptors to their original shape too
		{
			dup2(last_screen_out, 1);		
			output_redirection=false;
			output_redirection_index=0;
		}
		memset(&arr, 0, sizeof(arr));
	}
	destroy_space(command);				//at the end we should destroy the dynamic memory too
	return 0;	
}





