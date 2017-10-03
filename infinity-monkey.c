#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <string.h>

#define INTERFACE 0
#define GERADOR 1


int pesquisar(char *entrada) {
	FILE *fp = fopen("infinity-monkey.txt", "r");

	char letra;
	int ponteiro = 0;
	int pos = 0;
	while(fscanf(fp, "%c", &letra) != EOF) {
		ponteiro++;
		if(letra == entrada[0]) {
			pos = ponteiro;
			int i;
			for(i = 1; i < strlen(entrada) + 1; i++) {
				if(i == strlen(entrada)) {
					return pos;
				}

				if(fscanf(fp, "%c", &letra) == EOF) {
					return -1;
				}
				ponteiro++;
				if(letra != entrada[i]) {
					break;
				}
			}
		}
	}
}


void interface() {
	char *entrada = (char *) malloc(sizeof(char) * 255);
	while(1) {
		printf("O que deseja procurar? 0 para sair\n");
		scanf("%s", entrada);

		if(strcmp(entrada, "0") == 0) {
			return;
		}

		MPI_Request req;
		int para = 1, continua = 1;
		MPI_Send(&para, 1, MPI_INT, GERADOR, 1, MPI_COMM_WORLD);

		// Pesquisar
		int pos = pesquisar(entrada);
		printf("Encontrado na posicao %d\n", pos);

		MPI_Send(&continua, 1, MPI_INT, GERADOR, 1, MPI_COMM_WORLD);
	}
}


void gerador() {
	FILE *fp = fopen("infinity-monkey.txt", "w");

	MPI_Request req = (MPI_Request) malloc(sizeof(MPI_Request));// = MPI_REQUEST_NULL;
	int mensagem = 0;
	int flag;
	MPI_Status status;
	MPI_Irecv(&mensagem, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &req);
	while(1) {
		char letra = rand() % 27;
		if(letra == 26) {
			letra = ' ';
		} else {
			letra = letra + 97;
		}

		fprintf(fp, "%c", letra);

		MPI_Test(&req, &flag, &status);
		if(req == MPI_REQUEST_NULL) {
			if(mensagem) {
				// Reinicia mensagem
				mensagem = 0;

				// Fecha arquivo
				fclose(fp);

				// Espera mensagem pra continuar
				int continua;
				MPI_Recv(&continua, 1, MPI_INT, INTERFACE, 1, MPI_COMM_WORLD, &status);

				// Continua
				fp = fopen("infinity-monkey.txt", "a");
			} else {
				MPI_Irecv(&mensagem, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &req);
			}
		}
	}
}


int main(void) {
	MPI_Init(NULL, NULL);

	int n_processos;
	MPI_Comm_size(MPI_COMM_WORLD, &n_processos);
	if(n_processos != 2) {
		printf("Numero de processos deve ser 2\n");
		return 1;
	}

	int id_processo;
	MPI_Comm_rank(MPI_COMM_WORLD, &id_processo);

	if(id_processo == INTERFACE) {
		interface();
	} else if(id_processo == GERADOR) {
		gerador();
	}

	return 0;
}