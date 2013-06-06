#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "list.h"

#define PORT 10000
#define BACKLOG 10 // El número de conexiones permitidas
#define MAXDATASIZE 1000

typedef struct {
	int fd;
	char host[120];
} Banco;

static List *onlineAtm;
static List *onlineBanco;


// Se va a necesitar un mutex por cada lista

static pthread_mutex_t onlineAtmMutex;
static pthread_mutex_t onlineBancoMutex;

int autorizarEntidad(char *authFile, char *datos1, char *datos2) {
	int autorizado = 0;
	FILE *fp = fopen(authFile, "r");
	char user[10];
	char pass[40];
	printf("usuario: %s - pass: %s\n",datos1,datos2);
	while (fscanf(fp, "%s %s", user, pass) != EOF) {
		if ((strcmp(datos1, user) == 0) && (strcmp(datos2, pass) == 0)) {
			autorizado = 1;
		}

	}
	fclose(fp);
	return autorizado;
}

int autorizarBanco(char *datos1, char *datos2) {
	return autorizarEntidad("./banco.txt", datos1, datos2);
}

int autorizarAtm(char *datos1, char *datos2) {
	return autorizarEntidad("./atm.txt", datos1, datos2);
}

char* getBankStringFromList(List *list, char *data) {
	Node *node = list->first;
	while(node != NULL) {
		if (strncmp(node->data, data,4) == 0) {
			return node->data;
		}
		node = node->next;
	}
	return NULL;
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


 void enviarBanco(char *operacion, char *datos1, char *datos2, char *output) {
	int numbytes;
	char buf[MAXDATASIZE];
	//static char bufaux[MAXDATASIZE];
	char datos[5];
	char ip[120];
	int port=0;
	int fd;

	pthread_mutex_lock(&onlineBancoMutex);
	sscanf(getBankStringFromList(onlineBanco, datos1), "%s %s %d", datos, ip, &port);
	pthread_mutex_unlock(&onlineBancoMutex);
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

	strcpy(output,buf);
	close(fd);
}



void *hablar(void *conexionAsPointer) {
	int numbytes;
	char operacion[MAXDATASIZE];
	char datos1[MAXDATASIZE];
	char datos2[MAXDATASIZE];
	char aux[100];
	char aux2[100];
	char aux3[1000];
	char buffer[6];
	Banco *conexion = (Banco *) conexionAsPointer;

	if ((numbytes = recv(conexion->fd, operacion, MAXDATASIZE, 0)) == -1) {
		printf("Error en recv() \n");
		exit(-1);
	}

	operacion[numbytes] = '\0';

	send(conexion->fd, "ok", 3, 0);

	if ((numbytes = recv(conexion->fd, datos1, MAXDATASIZE, 0)) == -1) {
		printf("Error en recv() \n");
		exit(-1);
	}

	datos1[numbytes] = '\0';

	send(conexion->fd, "ok", 3, 0);

	if ((numbytes = recv(conexion->fd, datos2, MAXDATASIZE, 0)) == -1) {
		printf("Error en recv() \n");
		exit(-1);
	}

	datos2[numbytes] = '\0';

	if (strcmp(operacion, "authatm") == 0) {
		sprintf(aux, "%d", autorizarAtm(datos1, datos2));
		send(conexion->fd, aux, 2, 0);
		if (strcmp(aux,"1")==0) {
			pthread_mutex_lock(&onlineAtmMutex);
			if (!isInList(onlineAtm,datos1)){
				addToList(onlineAtm, datos1);
			}
			pthread_mutex_unlock(&onlineAtmMutex);
		}
	};

	if (strcmp(operacion, "deslogatm") == 0) {
		send(conexion->fd, "1", 2, 0);
		pthread_mutex_lock(&onlineAtmMutex);
		removeFromList(onlineAtm,datos1);
		pthread_mutex_unlock(&onlineAtmMutex);
	};

	if (strcmp(operacion, "authtarj") == 0) {
		enviarBanco(operacion, datos1, datos2, aux2);
		send(conexion->fd, aux2, 5, 0);
	};

	if (strcmp(operacion, "extraer") == 0) {
		enviarBanco(operacion, datos1, datos2, aux2);
		send(conexion->fd, aux2, 5, 0);
	};

	if (strcmp(operacion, "depositar") == 0) {
		enviarBanco(operacion, datos1, datos2, aux2);
		send(conexion->fd, aux2, 5, 0);
	};

	if (strcmp(operacion, "consulta") == 0) {
		enviarBanco(operacion, datos1, datos2, aux2);
		send(conexion->fd, aux2, 100, 0);
	};

	if (strcmp(operacion, "listado") == 0) {
		enviarBanco(operacion, datos1, datos2, aux3);
		send(conexion->fd, aux3, 1000, 0);
	};

	if (strcmp(operacion, "authbanco") == 0) {
		sprintf(aux, "%d", autorizarBanco(datos1, datos2));
		send(conexion->fd, aux, 5, 0);

		if (strcmp(aux, "1") == 0){
			if ((numbytes = recv(conexion->fd, buffer, MAXDATASIZE, 0)) == -1) {
				printf("Error en recv() \n");
				exit(-1);
			}

			pthread_mutex_lock(&onlineBancoMutex);
			char *bankStr = getBankStringFromList(onlineBanco, datos1);
			if (bankStr != NULL){
				removeFromList(onlineBanco, bankStr);
			}
			sprintf(aux, "%s %s %s", datos1, conexion->host, buffer);
			addToList(onlineBanco, aux);
			pthread_mutex_unlock(&onlineBancoMutex);

		}
	};

	if (strcmp(operacion, "deslogbanco") == 0) {
		send(conexion->fd, "1", 2, 0);
		pthread_mutex_lock(&onlineBancoMutex);
		removeFromList(onlineBanco,datos1);
		pthread_mutex_unlock(&onlineBancoMutex);
	};

	close(conexion->fd);

	return 0;
}

void *escuchar(void *parametro) {
	int fd, fd2;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t sin_size;
	int yes = 1;
	pthread_t hablar_thread;
	Banco datosConexion;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error en socket()\n");
		exit(-1);
	}

	server.sin_family = AF_INET;

	server.sin_port = htons(PORT);

	server.sin_addr.s_addr = INADDR_ANY;

	bzero(&(server.sin_zero), 8);

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	if (bind(fd, (struct sockaddr*) &server, sizeof(struct sockaddr)) == -1) {
		printf("error en bind() \n");
		exit(-1);
	}

	if (listen(fd, BACKLOG) == -1) {
		printf("error en listen()\n");
		exit(-1);
	}

	while (1) {
		sin_size = sizeof(struct sockaddr_in);

		if ((fd2 = accept(fd, (struct sockaddr *) &client, &sin_size)) == -1) {
			printf("error en accept()\n");
			exit(-1);
		}

		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(client.sin_addr.s_addr), ip, INET6_ADDRSTRLEN);

		datosConexion.fd=fd2;
		strcpy(datosConexion.host,ip);

		if (pthread_create(&hablar_thread, NULL, hablar, (void *)&datosConexion) < 0) {
			perror("No se creo el thread");
			pthread_exit((void *)1);
		};
	};

	return 0;
}



void mostrarPantallaInicial() {
	printf("1. Listar bancos conectados\n\n");
	printf("2. Listar atm conectados\n\n");
	printf("3. Listar en pantalla archivo de log\n\n");
	printf("4. Cerrar switch\n\n");
}

void listarAtm() {
	Node *node = onlineAtm->first;
	pthread_mutex_lock(&onlineAtmMutex);
	while(node != NULL) {
		printf("%s\n",node->data);
		node = node->next;
	}
	pthread_mutex_unlock(&onlineAtmMutex);
}

void listarBancos() {
	char datos[5];
	char ip[120];
	int port=0;
	Node *node = onlineBanco->first;

	pthread_mutex_lock(&onlineBancoMutex);
	while(node != NULL) {
		sscanf(node->data, "%s %s %d", datos, ip, &port);
		printf("%s\n",datos);
		node = node->next;
	}
	pthread_mutex_unlock(&onlineBancoMutex);
}

void initMutexes() {
	pthread_mutex_init(&onlineAtmMutex, NULL);
	pthread_mutex_init(&onlineBancoMutex, NULL);
}

int main() {
	char opcion;
	pthread_t escuchar_thread;

	onlineAtm = createList();
	onlineBanco = createList();
	initMutexes();
	if (pthread_create(&escuchar_thread, NULL, escuchar, NULL) < 0) {
		perror("No se creo el thread");
		return 1;
	};

	while (opcion != '4') {
		opcion = '0';
		mostrarPantallaInicial();
		scanf("%s", &opcion);
		switch (opcion) {
		case '1':
			listarBancos();
			break;
		case '2':
			listarAtm();
			break;
		case '3':
			printf("Listar log\n");
			break;
		case '4':
			printf("Cerrando el switch\n");
			break;
		}
	}
	return 0;
}
;
