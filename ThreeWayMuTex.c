/* By Xin Li at Oregon State University */

/* Three way mutual exlusion problem. */
/* Create NS searchers are always searching every 1-5s at random. */
/* N Inserters insert every 1-10s at random. */
/* N deleters delete every 1-10s random of time. */

#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<math.h>
#include<time.h>
#include<semaphore.h>

#define N 3
#define NS 10

typedef enum{ INSERT, SEARCH, DELETE } checker_t;
typedef enum{ ON, OFF } state;

sem_t sem_search;
sem_t sem_delete;
sem_t sem_insert;
state state_delete = OFF;
state state_insert = OFF;
state state_search = OFF;

int delete_ID[N] = { 0, 1, 2 }; 
int insert_ID[N] = { 0, 1, 2 }; 
int search_ID[NS]; 

int search_count = 0;
int insert_count = 0;
int delete_count = 0;

unsigned int fib(unsigned int n)
{
	return n < 2 ? n : fib(n - 1) + fib(n - 2); 
}

void check(checker_t checker)                                          /* My check function */
{
	switch (checker)
	{
	case INSERT:
		if (state_delete == OFF && state_insert == OFF)            /* Inserter can only work with search. */
		{	
			sem_post(&sem_insert);
		}
		break;

	case SEARCH:                          /* Searcher can work parallelly with search and insert, but cannot work with delete. */
		if (state_delete == OFF)
		{	
			sem_post(&sem_search);
		}
		break;

	case DELETE:						  /* Deleter can work with nobody. */
		if (state_insert == OFF && search_count == 0)
		{
			sem_post(&sem_delete);
		}
		break;
	}
}

void* searcher(void* id)
{
	int *ID = (int*)id;

	while (1)
	{
		int sleep_time = (rand() % 5 >= 0) ? (rand() % 5) : -(rand() % 5); 
		sleep(sleep_time);
		
		printf("[SEARCHER %d] WAKE UP \n"
			"[THREADS] %d SEARCH | %d INSERT | %d DELETE \n\n", *ID, search_count, insert_count, delete_count);	
		if(state_delete == OFF)
		{
			sem_post(&sem_search);
		}
		else
			printf("[SEARCHER %d] WAIT FOR DELETER \n\n", *ID);
				
		sem_wait(&sem_search);
		printf("[SEARCHER %d] SAFE FOR SEARCH \n"
			"[THREADS] %d SEARCH | %d INSERT | %d DELETE \n\n", *ID, search_count, insert_count, delete_count);

		search_count++; 

		/* Search for a random number. */
		printf("[SEARCHER %d] LOOKING UP\n"
			"[THREADS] %d SEARCH | %d INSERT | %d DELETE \n\n", *ID, search_count, insert_count, delete_count);
		
		fib(20);		

		search_count--; 
		check(DELETE);
	}

	return NULL;
}


void* deleter(void* id)
{
	int* ID = (int*)id;
		
	while (1)
	{
		int sleep_time = ((rand() % 10) >= 0) ? (rand() % 10) : -(rand() % 10); /*Generate random number*/
		sleep(sleep_time);

		printf("[DELETER %d] WAKE UP \n"
			"[THREADS] %d SEARCH | %d INSERT | %d DELETE \n\n",*ID, search_count, insert_count, delete_count);
		
		if(search_count == 0 && state_insert == OFF)
		{
			sem_post(&sem_delete);
			pthread_mutex_unlock(&muTex_delete);
		}
		else 
			printf("[DELETER %d] WAIT FOR SEARCHER OR INSERTER \n \n", *ID);
				
		sem_wait(&sem_delete);	
		state_delete = ON;
		delete_count++; 

		/* Delete a random number. */
		printf("[DELETER %d] DELETE ONE ITEM \n"
			"[THREADS] %d SEARCH | %d INSERT | %d DELETE \n\n", *ID, search_count, insert_count, delete_count);

		fib(20);		

		delete_count--;
		state_delete = OFF;
		
		check(INSERT);
		check(SEARCH);
	}
	
	return NULL;
}

void* inserter(void* id)
{
	int *ID = (int*)id;

	while (1)
	{
		int sleep_time = ((rand() % 5) >= 0) ? (rand() % 10) : -(rand() % 10); /*Generate random number*/
		sleep(sleep_time);

		printf("[INSERTER %d] WAKE UP \n"
			"[THREADS] %d SEARCH | %d INSERT | %d DELETE \n\n", *ID, search_count, insert_count, delete_count);
	
		if (state_insert == OFF && state_delete == OFF)
		{	
			sem_post(&sem_insert);
		}
		else
			printf("[INSERTER %d] WAIT FOR OTHER INSERTER OR DELETER \n\n", *ID);
				
		sem_wait(&sem_insert);
		state_insert = ON;
		insert_count++;

		printf("[INSERTER %d] ADD A NEW \n"
			"[THREADS] %d SEARCH | %d INSERT | %d DELETE \n\n", *ID, search_count, insert_count, delete_count);
		fib(20);		

		insert_count--;
		state_insert = OFF;
		
		check(DELETE);
		check(INSERT);
	}

	return NULL;
}

/* The main function for start everything. */
int main(int argc, char *argv[])
{
	/* Variable definition. */
	pthread_t inst[N];
	pthread_t srch[NS];
	pthread_t delt[N];

	sem_init(&sem_search, 0, 0);
	sem_init(&sem_insert, 0, 0);
	sem_init(&sem_delete, 0, 0);

	for(int i = 0; i < NS; i++)
	{
		search_ID[i] = i;
	}

	/* Threads Activation */
	for (int i = 0; i < N; i++)
	{
		int pthread_create_err = pthread_create(&inst[i], NULL, inserter, (void*)&insert_ID[i]);
		if (pthread_create_err != 0)	/* When initialize the thread unsuccessfully. */
		{
			perror("pthread_create()");
		}

		pthread_create_err = pthread_create(&delt[i], NULL, deleter, (void*)&delete_ID[i]);
		if (pthread_create_err != 0)	/* When initialize the thread unsuccessfully. */
		{
			perror("pthread_create()");
		}
	}

	for(int i = 0; i < NS; i++)
	{
		int pthread_create_err = pthread_create(&srch[i], NULL, searcher, (void*)&search_ID[i]);
		if(pthread_create_err != 0)
		{
			perror("pthread_create()");
		}	
	}

	for (int i = 0; i < N; i++)
	{
		pthread_join(inst[i], NULL);
		pthread_join(delt[i], NULL);
	}
	
	for(int i = 0; i < NS; i++)
		pthread_join(srch[i], NULL);
}
