/*
 ============================================================================
 Name        : auth-trx.c
 Author      : Diego Rodriguez
 Version     :
 Copyright   : 
 Description : Aplicativo del banco
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

#define DEFAULT_SWITCH_HOST "localhost"
#define DEFAULT_SWITCH_PORT 10000

#define DEFAULT_BANK_PORT 10006

#define BUFF_DEFAULT_SIZE 100

#define BIG_BUFF_DEFAULT_SIZE 1024

#define TEMP_FILE_PATH "./.default_filetmp.txt"

typedef struct {
	long id;
	char password[40];
} BankAuthorizer;

typedef struct {
	char number[21];
	char password[9];
} CreditCard;

typedef struct {
	int fd;
	int port;
	char host[120];
} Switch;

static int keepRunning = 1;

void intHandler(int signo) {
	if (signo == SIGINT) {
		keepRunning = 0;
	}
}

void exitWithError() {
	perror("Error");
	exit(EXIT_FAILURE);
}

void initUser(BankAuthorizer *user) {
	user->id = -1;
}

void initSwitch(Switch *switchTrans) {
	switchTrans->fd = -1;
	strcpy(switchTrans->host, DEFAULT_SWITCH_HOST);
	switchTrans->port = DEFAULT_SWITCH_PORT;
}

int authenticateBank(Switch *switchTrans, BankAuthorizer *user) {
	printf("Usuario: %ld\n", user->id);
	printf("Password: %s\n", user->password);
	char buffer[BUFF_DEFAULT_SIZE];

	if (write(switchTrans->fd, "authbanco", 16) < 0) {
		exitWithError();
	}
	if (read(switchTrans->fd, buffer, BUFF_DEFAULT_SIZE) < 0) {
		exitWithError();
	}
	sprintf(buffer, "%ld", user->id);
	if (write(switchTrans->fd, buffer, strlen(buffer) + 1) < 0) {
		exitWithError();
	}
	if (read(switchTrans->fd, buffer, BUFF_DEFAULT_SIZE) < 0) {
		exitWithError();
	}
	if (write(switchTrans->fd, user->password, strlen(user->password) + 1)
			< 0) {
		exitWithError();
	}
	if (read(switchTrans->fd, buffer, BUFF_DEFAULT_SIZE) < 0) {
		exitWithError();
	}
	return atoi(buffer);
}

int isCreditCardAndPassCorrect(char *cardInfo, CreditCard *creditCard) {
	char *s = strtok(cardInfo, ":\n");
	if (strcmp(creditCard->number, s) != 0) {
		return 0;
	}
	s = strtok(NULL, ":\n");
	if (strcmp(creditCard->password, s) == 0) {
		return 1;
	}
	return 0;
}

int isValidCreditCard(CreditCard *creditCard) {
	char buffer[100];
	int read;
	FILE *fp;
	if ((fp = fopen(
			"./accounts.txt",
			"r")) == NULL) {
		exitWithError();
	}
	while (fgets(buffer, BUFF_DEFAULT_SIZE, fp) != NULL) {
		if (buffer != NULL) {
			if (strncmp(creditCard->number, buffer, strlen(creditCard->number))
					== 0) {
				return isCreditCardAndPassCorrect(buffer, creditCard);
			}
		}
	}

	return 0;
}

void getAuthorizer(BankAuthorizer *user) {
	if (user->id == -1) {
		printf("Usuario:\n");
		scanf("%ld", &user->id);
		printf("Password:\n");
		scanf("%s", user->password);
	}
}

void sendServicePortToSwitch(Switch *switchTrans) {
	char buffer[BUFF_DEFAULT_SIZE];
	sprintf(buffer, "%d", DEFAULT_BANK_PORT);
	if (write(switchTrans->fd, buffer, strlen(buffer) + 1) < 0) {
		exitWithError();
	}
}

void connectToSwitch(Switch *switchTrans, BankAuthorizer *autorizer) {
	struct sockaddr_in switchAddress;
	struct hostent *switchHost;

	// creo el scoket para conectarme
	switchTrans->fd = socket(AF_INET, SOCK_STREAM, 0);

	if (switchTrans->fd < 0) {
		exitWithError();
	}

	// La dirección a la cual conectarse
	switchHost = gethostbyname(switchTrans->host);
	switchAddress.sin_family = AF_INET;
	switchAddress.sin_port = htons(switchTrans->port);

	bcopy((char *) switchHost->h_addr, (char *) &switchAddress.sin_addr.s_addr,
			switchHost->h_length);

	// me conecto al server
	if (connect(switchTrans->fd, (struct sockaddr*) &switchAddress,
			(socklen_t) sizeof(struct sockaddr_in)) < 0) {
		puts("Error conectandose al servidor");
		exitWithError();
	}

	if (!authenticateBank(switchTrans, autorizer)) {
		printf("Error de autenticación");
		exit(EXIT_FAILURE);
	}

	sendServicePortToSwitch(switchTrans);
}

float checkBalance(CreditCard *creditCard) {
	FILE *fp;
	char buffer[BUFF_DEFAULT_SIZE];
	char path[BUFF_DEFAULT_SIZE];
	sprintf(path, "./%s",
			creditCard->number);
	if ((fp = fopen(path, "r")) == NULL) {
		//exitWithError();
		return -1;
	}
	if (fgets(buffer, BUFF_DEFAULT_SIZE, fp) < 0) {
		exitWithError();
	}
	char *s = strtok(buffer, ":\n");
	s = strtok(NULL, ":\n");
	s = strtok(NULL, ":\n");

	return atof(s);
}

int addInFileAndClose(char *firstLine, char *filename) {
	FILE *fptmp;
	FILE *fp;
	char buffer[BUFF_DEFAULT_SIZE];

	if ((fptmp = fopen(TEMP_FILE_PATH, "w")) == NULL) {
		exitWithError();
	}
	if ((fp = fopen(filename, "r")) == NULL) {
		exitWithError();
	}
	if (fputs(firstLine, fptmp) == EOF) {
		exitWithError();
	}
	while (fgets(buffer, BUFF_DEFAULT_SIZE, fp) != NULL) {
		fputs(buffer, fptmp);
	}
	fclose(fptmp);
	fclose(fp);
	if (remove(filename) != 0) {
		exitWithError();
	}
	if (rename(TEMP_FILE_PATH, filename) < 0) {
		exitWithError();
	}
	return 0;
}

int changeAmmount(CreditCard *creditCard, float ammout) {
	FILE *fp;
	char buffer[BUFF_DEFAULT_SIZE];
	char path[BUFF_DEFAULT_SIZE];
	float balance = 0;

	sprintf(path, "./%s",
			creditCard->number);
	if ((fp = fopen(path, "r")) == NULL) {
		//exitWithError();
		return -1;
	}
	if (fgets(buffer, BUFF_DEFAULT_SIZE, fp) < 0) {
		exitWithError();
	}
	char *s = strtok(buffer, ":\n");
	s = strtok(NULL, ":\n");
	s = strtok(NULL, ":\n");
	balance = atof(s);
	if (balance + ammout < 0) {
		return -1;
	}
	sprintf(buffer, "%s:%.2f:%.2f\n", ammout >= 0 ? "CRE" : "DEB", ammout,
			balance + ammout);
	fclose(fp);
	return addInFileAndClose(buffer, path);
}

int fillBufferWithMovements(CreditCard *creditCard, char *out) {
	FILE *fp;
	char buffer[BUFF_DEFAULT_SIZE];
	char path[BUFF_DEFAULT_SIZE];

	sprintf(path, "./%s",
			creditCard->number);
	if ((fp = fopen(path, "r")) == NULL) {
		//exitWithError();
		return -1;
	}
	out[0] = '\0';
	int lines = 0;
	while(fgets(buffer,BUFF_DEFAULT_SIZE, fp) != NULL && lines++ < 10) {
		strcat(out, buffer);
	}
	fclose(fp);

	return 0;
}

void *hablar(int fd) {
	int numbytes;
	char operacion[BUFF_DEFAULT_SIZE];
	char datos1[BUFF_DEFAULT_SIZE];
	char datos2[BUFF_DEFAULT_SIZE];
	char bigBuff[BIG_BUFF_DEFAULT_SIZE];

	if ((numbytes = recv(fd, operacion, BUFF_DEFAULT_SIZE, 0)) == -1) {
		printf("Error en recv() \n");
		exitWithError();
	}

	operacion[numbytes] = '\0';

	send(fd, "ok", 3, 0);

	if ((numbytes = recv(fd, datos1, BUFF_DEFAULT_SIZE, 0)) == -1) {
		printf("Error en recv() \n");
		exitWithError();
	}

	datos1[numbytes] = '\0';

	send(fd, "ok", 3, 0);

	if ((numbytes = recv(fd, datos2, BUFF_DEFAULT_SIZE, 0)) == -1) {
		printf("Error en recv() \n");
		exit(-1);
	}

	datos2[numbytes] = '\0';

	if (strcmp(operacion, "authtarj") == 0) {
		CreditCard card;
		strcpy(card.number, datos1);
		strcpy(card.password, datos2);
		if (isValidCreditCard(&card)) {
			send(fd, "1", 2, 0);
		} else {
			send(fd, "0", 2, 0);
		}
	};

	if (strcmp(operacion, "extraer") == 0) {
		float ammount = 0;
		CreditCard card;
		strcpy(card.number, datos1);
		ammount = atof(datos2);
		if (changeAmmount(&card, -ammount) == 0) {
			send(fd, "1", 2, 0);
		} else {
			send(fd, "0", 2, 0);
		}

	};

	if (strcmp(operacion, "depositar") == 0) {
		float ammount = 0;
		CreditCard card;
		strcpy(card.number, datos1);
		ammount = atof(datos2);
		if (changeAmmount(&card, ammount) != 0) {
			send(fd, "1", 2, 0);
		} else {
			send(fd, "0", 2, 0);
		}
	};

	if (strcmp(operacion, "consulta") == 0) {
		CreditCard card;
		strcpy(card.number, datos1);
		strcpy(card.password, datos2);
		float balance = checkBalance(&card);
		char buffer[BUFF_DEFAULT_SIZE];
		sprintf(buffer, "%.2f", balance);
		send(fd, buffer, 10, 0);
	};

	if (strcmp(operacion, "listado") == 0) {
		CreditCard card;
		int sent = 0;
		strcpy(card.number, datos1);
		fillBufferWithMovements(&card, bigBuff);
		send(fd, bigBuff, strlen(bigBuff), 0);
		while (sent < strlen(bigBuff)) {
			sent += send(fd, bigBuff+sent, strlen(bigBuff) - sent, 0);
		}
	};

	close(fd);

	return 0;
}

void processTransactions() {
	int sockopt = 1;
	int fd, fd2; /* los ficheros descriptores */

	struct sockaddr_in server;
	/* para la información de la dirección del servidor */

	struct sockaddr_in client;
	/* para la información de la dirección del cliente */

	socklen_t sin_size;

	/* A continuación la llamada a socket() */
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		exitWithError();
	}

	server.sin_family = AF_INET;

	server.sin_port = htons(DEFAULT_BANK_PORT);

	server.sin_addr.s_addr = INADDR_ANY;
	/* INADDR_ANY coloca nuestra dirección IP automáticamente */

	bzero(&(server.sin_zero), 8);
	/* escribimos ceros en el reto de la estructura */

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) == -1) {
		exitWithError();
	}

	/* A continuación la llamada a bind() */
	if (bind(fd, (struct sockaddr*) &server, sizeof(struct sockaddr)) == -1) {
		exitWithError();
	}

	if (listen(fd, 5) == -1) { /* llamada a listen() */
		exitWithError();
	}

	if (signal(SIGINT, intHandler) == SIG_ERR) {
		exitWithError();
	}

	while (keepRunning) {
		sin_size = sizeof(struct sockaddr_in);
		/* A continuación la llamada a accept() */
		if ((fd2 = accept(fd, (struct sockaddr *) &client, &sin_size)) == -1) {
			exitWithError();
		}

		hablar(fd2);

		close(fd2); /* cierra fd2 */
	}

	printf("deslogeandose\n");
}

int main(int argc, char *argv[]) {
	char c;
	BankAuthorizer user;
	Switch switchTrans;

	initUser(&user);
	initSwitch(&switchTrans);

	while ((c = getopt(argc, argv, "u:c:h:p:l:")) != -1)
		switch (c) {
		case 'u':
			user.id = atol(optarg);
			break;
		case 'c':
			strcpy(user.password, optarg);
			break;
		case 'h':
			printf("Host: %s\n", optarg);
			strcpy(switchTrans.host, optarg);
			break;
		case 'p':
			printf("Puerto: %s\n", optarg);
			switchTrans.port = atoi(optarg);
			break;
		case 'l':
			printf("Logfile: %s\n", optarg);
			break;
		case '?':

		default:
			printf("Uso: auth-trx bla bla");
			break;
		}
	getAuthorizer(&user);
	connectToSwitch(&switchTrans, &user);
	close(switchTrans.fd);
	processTransactions();

	return EXIT_SUCCESS;
}
