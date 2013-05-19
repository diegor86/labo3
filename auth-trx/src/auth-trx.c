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

int main(int argc, char *argv[]) {
	char c;
    char *cvalue = NULL;

    while ((c = getopt (argc, argv, "u:c:h:p:l:")) != -1)
      switch (c) {
        case 'u':
        	printf("Usuario: %s\n", optarg);
          break;
        case 'c':
        	printf("Password: %s\n", optarg);
          break;
        case 'h':
        	printf("Host: %s\n", optarg);
          break;
        case 'p':
        	printf("Puerto: %s\n", optarg);
        	break;
        case 'l':
        	printf("Logfile: %s\n", optarg);
        	break;
        case '?':

        default:
          printf("Uso: auth-trx bla bla");
          break;
        }
    return EXIT_SUCCESS;
}
