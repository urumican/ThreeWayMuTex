/* Three way mutual exlusion problem. */
/* Create 3 searcher always searching. */
/* 3 Inserter insert every 2 seconds. */
/* 3 deleter delete every random of time. */
 
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<math.h>
#include<time.h>
#include<semaphore.h>
 
#define N 3
#define NS 5
 
typedef enum{ INSERT, SEARCH, DELETE } checker_t;
typedef enum{ ON, OFF } state;

pthread_mutex_t muTex_search;
pthread_mutex_t muTex_delete;
pthread_mutex_t muTex_insert;

state state_delete = OFF;
state state_insert = OFF;
state state_search = OFF;

int delete_ID[N] = { 0, 1, 2 }; 
int insert_ID[N] = { 0, 1, 2 }; 
int search_ID[NS]; 

int search_count = 0;

 
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
			//pthread_mutex_unlock(&muTex_insert);
			sem_post(&sem_insert);
		}
		break;
 
	case SEARCH:                          /* Searcher can work parallelly with search and insert, but cannot work with delete. */
		if (state_delete == OFF)
		{	
			//pthread_mutex_unlock(&muTex_search);
			sem_post(&sem_search);
		}
		break;
 
	case DELETE:						  /* Deleter can work with nobody. */
		if (state_insert == OFF && search_count == 0)
		{
			//pthread_mutex_unlock(&muTex_delete);
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
		int sleep_time = (rand() % 10 >= 0) ? (rand() % 10) : -(rand() % 10); 
		sleep(sleep_time);
		
		if(state_delete == OFF)
		{
			sem_post(&sem_search);
			//pthread_mutex_unlock(&muTex_search);
		}
		else
			printf("[SEARCHER %d] WAIT FOR DELETER \n\n", *ID);
 
		//pthread_mutex_lock(&muTex_search);
		sem_wait(&sem_search);
		printf("[SEARCHER %d] SAFE FOR SEARCH \n\n", *ID);
		search_count++; 
 
		/* Search for a random number. */
		printf("[SEARCHER %d] LOOKING UP, TOTAL # OF SEARCHER: %d \n\n", *ID, search_count);
		
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
		
		//check(DELETE, *ID);
		if(search_count == 0 && state_insert == OFF)
		{
			sem_post(&sem_delete);
			pthread_mutex_unlock(&muTex_delete);
		}
		else 
			printf("[DELETER %d] WAIT FOR SEARCHER OR INSERTER \n \n", *ID); 
 
		//pthread_mutex_lock(&muTex_delete);
		sem_wait(&sem_delete);	
		state_delete = ON;
 
		/* Delete a random number. */
		printf("[DELETER %d] DELETE ONE ITEM \n\n", *ID);
		fib(20);		
 
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
 
		//check(INSERT, *ID);
		if (state_insert == OFF && state_delete == OFF)
		{
			//pthread_mutex_lock(&muTex_insert);
			sem_post(&sem_insert);
		}
		else
			printf("[INSERTER %d] WAIT FOR OTHER INSERTER OR DELETER \n\n", *ID);
 
		//pthread_mutex_lock(&muTex_insert);
		sem_wait(&sem_insert);
		state_insert = ON;
 
		printf("[INSERTER %d] ADD A NEW \n\n", *ID);
		fib(20);		
 
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
 
	/* MuTex initialization */
	//pthread_mutex_init(&muTex_insert, NULL);
	//pthread_mutex_init(&muTex_delete, NULL);
	//pthread_mutex_init(&muTex_search, NULL);
 
	sem_init(&sem_search, 0, 1);
	sem_init(&sem_insert, 0, 1);
	sem_init(&sem_delete, 0, 1);
 
	for(int i = 0; i < NS; i++)
	{
		search_ID[i] = i;
		//printf("%d",i);
	}
	/* Threads Activation */
	
 
	for (int i = 0; i < N; i++)
	{
		int pthread_create_err = pthread_create(&inst[i], NULL, inserter, (void*)&insert_ID[i]);
		if (pthread_create_err != 0)	/* When initialize the thread unsuccessfully. */
		{
			perror("pthread_create()");
		}
 
	/*	pthread_create_err = pthread_create(&srch[i], NULL, searcher, (void*)&search_ID[i]);
		if (pthread_create_err != 0) */	/* When initialize the thread unsuccessfully. */
	/*	{
			perror("pthread_create()");
		}
	*/
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
		//pthread_join(srch[i], NULL);
		pthread_join(delt[i], NULL);
	}
	
	for(int i = 0; i < NS; i++)
		pthread_join(srch[i], NULL);
}
