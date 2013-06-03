#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define MAXDATASIZE 100


typedef struct {
	int user;
	char pass[9];
} atm;

typedef struct {
	char number[21];
	int password;
} tarjeta;

void printfSiInteractivo() {

}

int conexion(char *ip, int port){
	int fd;
	struct hostent *he;
	struct sockaddr_in server;
	if ((he=gethostbyname(ip))==NULL){
		printf("gethostbyname() error\n");
		exit(-1);
	}

	if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
		printf("socket() error\n");
		exit(-1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(server.sin_zero),8);

	if(connect(fd, (struct sockaddr *)&server,
		sizeof(struct sockaddr))==-1){
		printf("connect() error\n");
		exit(-1);
	}
	return fd;
}

char *hablar(char *ip, int port, char *operacion, char *datos1, char *datos2) {
	int numbytes;
	char buf[MAXDATASIZE];
	static char bufaux[MAXDATASIZE];
	int fd;
	fd = conexion(ip, port);

	send(fd,operacion,10,0);

	if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){
         printf("Error en recv() \n");
         exit(-1);
	}

	buf[numbytes]='\0';

	send(fd,datos1,20,0);

	if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){
		printf("Error en recv() \n");
		exit(-1);
	}

	buf[numbytes]='\0';

	send(fd,datos2,30,0);

	if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){
		printf("Error en recv() \n");
		exit(-1);
	}

	buf[numbytes]='\0';

	strcpy(bufaux,buf);
	close(fd);
	return bufaux;
}

int auntenticarAtm(char *ip, int port, atm user){
	char buffer[10];
	sprintf(buffer, "%d", user.user);
	return atoi(hablar(ip, port, "authatm", buffer, user.pass));
}

int deslogAtm(char *ip, int port, atm user){
	char buffer[10];
	sprintf(buffer, "%d", user.user);
	return atoi(hablar(ip, port, "deslogatm", buffer, user.pass));
}

int autenticarTarjeta(char *ip, int port, tarjeta user){
	char *buffer;
	char *buffer2;
	buffer = (char*)malloc(strlen("") * sizeof(char));
	buffer2 = (char*)malloc(strlen("") * sizeof(char));
	strcpy(buffer, user.number);
	sprintf(buffer2, "%d", user.password);
	return atoi(hablar(ip, port, "authtarj", buffer, buffer2));
}

int extraer(char *ip, int port, tarjeta user){
	float aux=0;
	char *buffer;
	char *buffer2;
	buffer = (char*)malloc(strlen("") * sizeof(char));
	buffer2 = (char*)malloc(strlen("") * sizeof(char));
	printf("Cuanto desea extraer? (formato xxxx.xx)");
	scanf("%f",&aux);
	strcpy(buffer, user.number);
	sprintf(buffer2, "%.2f", aux);
	return atoi(hablar(ip, port, "extraer", buffer, buffer2));
}

int depositar(char *ip, int port, tarjeta user){
	float aux=0;
	char *buffer;
	char *buffer2;
	buffer = (char*)malloc(strlen("") * sizeof(char));
	buffer2 = (char*)malloc(strlen("") * sizeof(char));
	printf("Cuanto desea depositar? (formato xxxx.xx)");
	scanf("%f",&aux);
	strcpy(buffer, user.number);
	sprintf(buffer2, "%.2f", aux);
	return atoi(hablar(ip, port, "depositar", buffer, buffer2));
}

float consulta(char *ip, int port, tarjeta user){
	float aux=0;
	char *buffer;
	char *buffer2;
	buffer = (char*)malloc(strlen("") * sizeof(char));
	buffer2 = (char*)malloc(strlen("") * sizeof(char));
	strcpy(buffer, user.number);
	sprintf(buffer2, "%.2f", aux);
	return atof(hablar(ip, port, "consulta", buffer, buffer2));
}

void movimientos(char *ip, int port, tarjeta user){

}

void meterEnLog(char *movimiento, char *log) {
	FILE *fp;
	fp = fopen ( log, "a" );
	fprintf(fp, "%s\n" ,movimiento);
	fclose (fp);
}

void mostrarPantallaInicial() {
	printf("1. Extracción de dinero en efectivo\n\n");
	printf("2. Depósito de dinero en efectivo\n\n");
	printf("3. Consulta de saldo\n\n");
	printf("4. Listado de los últimos movimientos\n\n");
	printf("5. Volver a la pantalla inicial\n\n");
}

int main( int argc, char *argv[] ){
	atm atm;
	tarjeta usuario;
	int port;
	int interactivo=1;
	int cerrar=0;
	char c;
	char opcion;
	char* ip;
	char* log;
	char* batchFile;
	char* buffer;
	ip = (char*)malloc(strlen("") * sizeof(char));
	log = (char*)malloc(strlen("") * sizeof(char));
	batchFile = (char*)malloc(strlen("") * sizeof(char));
	buffer = (char*)malloc(strlen("") * sizeof(char));
	atm.user=0;
	ip="localhost";
	port=10000;
	strcpy(atm.pass,"");
	strcpy(log, "./atm-");



	while ((c = getopt (argc, argv, "u:c:l:f:h:p:")) != -1) {
	      switch (c) {
		case 0:
			break;

		case 'u':
			atm.user=atoi(optarg);
			break;

		case 'c':
			strcpy(atm.pass,optarg);
			break;

		case 'l':
			strcpy(log,optarg);
			break;

		case 'f':
			strcpy(batchFile,optarg);
			interactivo=0;
			break;

		case 'h':
			strcpy(ip,optarg);
			break;

		case 'p':
			port=atoi(optarg);
			break;

		}
	}


	if (atm.user==0) {
		printf("Debe ingresar un usuario\n");
		scanf("%d",&atm.user);
	};

	if (strcmp(atm.pass,"")==0) {
		printf("Debe ingresar un password\n");
		scanf("%s",atm.pass);
	};

	if (strcmp(log,"./atm-")==0) {
		sprintf(buffer, "%d", atm.user);
		strcat(log, buffer);
		strcat(log, ".log");
	};


	if (auntenticarAtm(ip, port, atm)==1) {
		if (interactivo==1) {
			while (1) {
				printf("Ingrese numero de tarjeta\n");
				scanf("%s",usuario.number);
				printf("Ingrese su contraseña\n");
				scanf("%d",&usuario.password);
				if (autenticarTarjeta(ip, port, usuario)==1) {
					while (1) {
						opcion='0';
						cerrar=0;
						mostrarPantallaInicial();
						scanf("%s",&opcion);
						switch(opcion) {
							case '1':
								if (extraer(ip, port, usuario)==1){
									printf("Extracción exitosa\n");
								} else printf("No tiene saldo\n");
								break;
							case '2':
								if (depositar(ip, port, usuario)==1) {
									printf("Operación exitosa\n");
								} else printf("Hubo algun error\n");
								break;
							case '3':
								printf("Saldo: %.2f\n", consulta(ip, port, usuario));
								break;
							case '4':
								movimientos(ip, port, usuario);
								break;
							case '5':
								cerrar=1;
								break;
						}

						if (cerrar==1) break;
					}
				}
				else {
					if ((strcmp(usuario.number,"0")==0) && (usuario.password==0)) {
						if (deslogAtm(ip, port, atm)==1) {
							break;
						}
					}
				}
			}
		}
		else {
			printf("laburamos en batch\n");
		}
	}
	return 0;
}
