#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 2

typedef struct{
    int id;
    int state; // 0 = livre, 1 = processando, 2 = peçaPronta
} Maquina;

typedef struct{
    int state; // 0 = livre , 1 = peçaM1, 2 = peçaM2
} Robo;

typedef struct{
    int buffer[BUFFER_SIZE];
    int buffer_count;

    Maquina m1;
    Maquina m2;
    Robo r;

    pthread_mutex_t mutex; // evita colisões
    pthread_cond_t condMaquinas; // alarme de condição da maquina
    pthread_cond_t condRobo; // alarme de condição do robo
	pthread_cond_t condBuffer;
	pthread_cond_t ext_ev;
} Manufatura;

void *maquina1(void *arg){
    Manufatura *f = (Manufatura *)arg;
    while (1) {
        printf("[M1] PRODUZINDO PECA...\n");
        sleep(rand() % 3 + 1);  // Tempo de produção

		pthread_mutex_lock(&f->mutex);

		f->m1.state = 2;

		printf("[M1] PEÇA PRONTA \n");

		pthread_cond_signal(&f->condRobo);

		while(f->m1.state == 2){
			pthread_cond_wait(&f->condMaquinas, &f->mutex);
		}

		printf("[M1] TERMINOU A PRODUÇÃO.\n");
		pthread_mutex_unlock(&f->mutex);
    }
    return NULL;
}

void *maquina2(void *arg){
	Manufatura *f = (Manufatura *)arg;
    while (1){
        printf("[M2] PRODUZINDO PECA...\n");
        sleep(rand() % 3 + 1);  // Tempo de produção

		pthread_mutex_lock(&f->mutex);

		f->m2.state = 2;

		printf("[M2] PEÇA PRONTA \n");

		pthread_cond_signal(&f->condRobo);

		while(f->m2.state == 2){
			pthread_cond_wait(&f->condMaquinas, &f->mutex);
		}

		printf("[M2] TERMINOU A PRODUÇÃO.\n");
		pthread_mutex_unlock(&f->mutex);
    }
	return NULL;
}

void *robo(void *arg){
	Manufatura *f = (Manufatura *)arg;

	while (1){

		pthread_mutex_lock(&f->mutex);

		// robo carregando peça
		if(f->r.state != 0){

			// verifica se o buffer está cheio, caso estiver cheio, o robô espera
			if(f->buffer_count >= BUFFER_SIZE){
				printf("[R] BUFFER CHEIO.\n");
				pthread_cond_wait(&f->condBuffer, &f->mutex);
			}
		
			else{
				printf("[R] DEPOSITOU A PEÇA.\n");
				f->buffer_count++; // se tem vaga, recebe a peça
				f->r.state = 0; // robo livre novamente

				pthread_cond_signal(&f->condBuffer); // aviso de peça nova no buffer
				pthread_cond_broadcast(&f->condMaquinas); 
			}
		}
		else{
				if(f->m1.state == 2){
				printf("[R] PEGANDO MEÇA DA MÁQUINA 1...\n");
				f->r.state = 1; // estado carregando peça de m1
				f->m1.state = 0; // livre


				pthread_cond_broadcast(&f->condMaquinas);
				}
				else if(f->m2.state == 2){
					printf("[R] PEGANDO MEÇA DA MÁQUINA 2...\n");
					f->r.state = 2; // estado carregando peça de m2
					f->m2.state = 0; // livre

					pthread_cond_broadcast(&f->condMaquinas);
				}
				else{
					// nada pronto, espera
					printf("[R] SEM TAREFAS.\n");
					pthread_cond_wait(&f->condRobo, &f->mutex);
				}
		}
		pthread_mutex_unlock(&f->mutex);
	}
	return NULL;
}

void *cons(void *arg){
	Manufatura *f = (Manufatura *)arg;
	while(1){

		pthread_mutex_lock(&f->mutex);

		while(f->buffer_count == 0){
			pthread_cond_wait(&f->condBuffer, &f->mutex);
		}

		f->buffer_count--;

		printf("[C] CONSUMIU PEÇA. \n");

		pthread_cond_signal(&f->condBuffer);
		
		pthread_mutex_unlock(&f->mutex);

		sleep(rand() % 3 + 2);
	}
	return NULL;
} 

int main() {

    // definiçao dos ids de cada uma das máquinas

	Manufatura man;

    man.m1.id = 1;
    man.m2.id = 2; 

    // inicialização dos estados iniciais de cada uma das máquinas

    man.m1.state = 0;
    man.m2.state = 0;

    // inicialização do estado do robô
    man.r.state = 0;

    man.buffer_count = 0; // inicialização do buffer

	srand(time(NULL));

	pthread_mutex_init(&man.mutex, NULL);

	// inicialização
	pthread_cond_init(&man.condMaquinas, NULL);
	pthread_cond_init(&man.condRobo, NULL);
	pthread_cond_init(&man.condBuffer, NULL);
	pthread_cond_init(&man.ext_ev, NULL);

	pthread_t t_m1, t_m2, t_r, t_ext;

	// criação das threads
	pthread_create(&t_m1, NULL, maquina1, &man);
	pthread_create(&t_m2, NULL, maquina2, &man);
	pthread_create(&t_r, NULL, robo, &man);
	pthread_create(&t_ext, NULL, cons, &man);

	// 
	pthread_join(t_m1, NULL);
	pthread_join(t_m2, NULL);
	pthread_join(t_r, NULL);
	pthread_join(t_ext, NULL);

    return 0;
}
