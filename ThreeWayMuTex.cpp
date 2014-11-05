/* By Xin Li at Oregon State University */
 
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
 
#define N 3
 
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
int search_ID[N] = { 0, 1, 2 }; 
 
int search_count = 0;
 
void check(checker_t checker, int ID)                                          /* My check function */
{
	switch (checker)
	{
	case INSERT:
		if (state_delete == OFF && state_insert == OFF)            /* Inserter can only work with search. */
		{
			pthread_mutex_unlock(&muTex_insert);
		}
		else if (state_delete == ON)
		{
			printf("[INSERTER %d] WAIT FOR DELETER \n\n", ID);
		}
		else if (state_insert == ON)
		{
			printf("[INSERTER %d] WAIT FOR OTHER INSERTER \n\n", ID);
		}
 
		break;
 
	case SEARCH:                          /* Searcher can work parallelly with search and insert, but cannot work with delete. */
		if (state_delete == OFF)
		{
			pthread_mutex_unlock(&muTex_search);
		}
		else
		{
			printf("[SEARCHER %d] WAIT FOR DELETER \n\n", ID);
		}
 
		break;
 
	case DELETE:						  /* Deleter can work with nobody. */
		if (state_insert == OFF && search_count == 0)
		{
			pthread_mutex_unlock(&muTex_delete);
		}
		else if (state_insert == ON || search_count != 0)
		{
			printf("[DELETER %d] WAIT FOR SEARCH OR INSERT \n\n", ID);
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
 
		check(SEARCH, *ID);
 
		pthread_mutex_lock(&muTex_search);
		search_count += 1; 
		
		/* Search for a random number. */
		printf("[SEARCH %d] LOOKING UP \n\n", *ID);		
 
		search_count -= 1; 
	
		if(search_count == 0 && state_delete == OFF)
			pthread_mutex_unlock(&muTex_delete);	
 
	}
 
	return NULL;
}
 
 
void* deleter(void* id)
{
	int* ID = (int*)id;
		
	while (1)
	{
		int sleep_time = (rand() % 10 >= 0) ? (rand() % 10) : -(rand() % 10); /*Generate random number*/
		sleep(sleep_time);
		
		check(DELETE, *ID);
 
		pthread_mutex_lock(&muTex_delete);
		state_delete = ON;
 
		/* Delete a random number. */
		printf("[DELETER %d] DELETE ONE ITEM \n\n", *ID);
 
		state_delete = OFF;
	}
	
	return NULL;
}
 
void* inserter(void* id)
{
	int *ID = (int*)id;
 
	while (1)
	{
		int sleep_time = (rand() % 10 >= 0) ? (rand() % 10) : -(rand() % 10); /*Generate random number*/
		sleep(sleep_time);
 
		check(INSERT, *ID);
 
		pthread_mutex_lock(&muTex_insert);
		state_insert = ON;
 
		printf("[INSERT %d] ADD A NEW \n\n", *ID);
		
		state_insert = OFF;
	}
 
	return NULL;
}
 
/* The main function for start everything. */
int main(int argc, char *argv[])
{
	/* Variable definition. */
	pthread_t inst[N];
	pthread_t srch[N];
	pthread_t delt[N];
 
	/* MuTex initialization */
	pthread_mutex_init(&muTex_insert, NULL);
	pthread_mutex_init(&muTex_delete, NULL);
	pthread_mutex_init(&muTex_search, NULL);
 
	/* Threads Activation */
	
 
	for (int i = 0; i < N; i++)
	{
		int pthread_create_err = pthread_create(&inst[i], NULL, inserter, (void*)&insert_ID[i]);
		if (pthread_create_err != 0)	/* When initialize the thread unsuccessfully. */
		{
			perror("pthread_create()");
		}
 
		pthread_create_err = pthread_create(&srch[i], NULL, searcher, (void*)&search_ID[i]);
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
 
	for (int i = 0; i < N; i++)
	{
		pthread_join(inst[i], NULL);
		pthread_join(srch[i], NULL);
		pthread_join(delt[i], NULL);
	}
}
