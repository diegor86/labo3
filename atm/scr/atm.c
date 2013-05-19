#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main( int argc, char *argv[] ){
	int user, port;
	char c, pass[8];
	char* ip;
	ip = (char*)malloc(strlen("") * sizeof(char));
	char* log;
	log = (char*)malloc(strlen("./atm-") * sizeof(char));
	buffer = (char*)malloc(strlen("") * sizeof(char));
	user=0;
	port=0;
	strcpy(pass,"");
	//strcpy(ip,"");


	while ((c = getopt (argc, argv, "u:c:l:f:h:p:")) != -1) {
	      switch (c) {
		case 0:
			break;

		case 'u':
			user=atoi(optarg);
			break;

		case 'c':
			strcpy(pass,optarg);
			break;

		case 'l':

			break;

		case 'f':

			break;

		case 'h':
			strcpy(ip,optarg);
			break;

		case 'p':
			port=atoi(optarg);
			break;

		}
	}


	if (user==0) {
		printf("Debe ingresar un usuario\n");
		scanf("%d",&user);
	};

	if (strcmp(pass,"")==0) {
		printf("Debe ingresar un password\n");
		scanf("%s",pass);
	};

	if (strcmp(ip,"")==0) {
		printf("Debe ingresar un ip\n");
		scanf("%s",ip);
	};

	if (port==0) {
		printf("Debe ingresar un puerto\n");
		scanf("%d",&port);
	};

	//itoa(user,buffer,10);
	//strcat(log, &user);
	//strcat(log, ".log");

	printf("%d - %s\n",user, pass);
	printf("%s - %d\n",ip, port);
	printf("%s\n",log);

	return 0;
}
