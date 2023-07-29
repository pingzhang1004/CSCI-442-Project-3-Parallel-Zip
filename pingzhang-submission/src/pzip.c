#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pzip.h"

/**
 * pzip() - zip an array of characters in parallel
 *
 * Inputs:
 * @n_threads:		   The number of threads to use in pzip
 * @input_chars:		   The input characters (a-z) to be zipped
 * @input_chars_size:	   The number of characaters in the input file
 *
 * Outputs:
 * @zipped_chars:       The array of zipped_char structs
 * @zipped_chars_count:   The total count of inserted elements into the zippedChars array.
 * @char_frequency[26]: Total number of occurences
 *
 * NOTE: All outputs are already allocated. DO NOT MALLOC or REASSIGN THEM !!!
 *
 */

int chars_block_size_global =0;       //a global value for repprenst the block size of chars in each thread.
int zipped_counter_global =0;        //a global value for outputs (int zipped_chars_count)
int *frequency_global;              //a global value for outputs (int char frequency[26])
int n_threads_global = 0;          //a global value for repprenst the number of threads
pthread_barrier_t barrier;
pthread_mutex_t mutex_lock01 = PTHREAD_MUTEX_INITIALIZER;  
pthread_mutex_t mutex_lock02 = PTHREAD_MUTEX_INITIALIZER;  
struct zipped_char *zipped_char_global;   //a global value for outputs (zipped_char *zipped_char)
int input_chars_size_global;
int *combine_global;

// struct arg_thr_fn is the argument of the function *thr_fn(void *arg_thr_fn)
struct arg_local {
	int thIdD;
	int start_index;
	char *input_chars; //这里对吗？
};

void *counter_chars(void *arg); //the thread function

void pzip(int n_threads, char *input_chars, int input_chars_size, struct zipped_char *zipped_chars, int *zipped_chars_count, int *char_frequency)
{
	frequency_global = char_frequency;
	n_threads_global = n_threads;
	input_chars_size_global = input_chars_size;
	combine_global = malloc(n_threads * sizeof(int));	
	pthread_t threadsID_list[n_threads];
	pthread_barrier_init(&barrier, NULL, n_threads);
	chars_block_size_global = input_chars_size / n_threads;
	struct arg_local *arg_local;
	arg_local = malloc(n_threads * sizeof(struct arg_local)); 
	int max_output_size = input_chars_size_global * sizeof(struct zipped_char);
	zipped_char_global = malloc(max_output_size);
	//printf("zipped_char_global size in malloc:",sizeof(zipped_char_global)) ;
	for (int i = 0; i < n_threads; i++) {
		arg_local[i].thIdD = i;
		arg_local[i].start_index = i * chars_block_size_global;
		arg_local[i].input_chars = input_chars; 
		pthread_create(&threadsID_list[i], NULL, counter_chars, &(arg_local[i])); 
	}
	for (int i = 0; i < n_threads; i++) {
		pthread_join(threadsID_list[i], NULL); 
	}
    //printf("zipped_counter_global: %d\n",zipped_counter_global);
	for (int k = 0; k < zipped_counter_global; k++) {
		zipped_chars[k].character = zipped_char_global[k].character;
		zipped_chars[k].occurence = zipped_char_global[k].occurence;
	}
	*zipped_chars_count = zipped_counter_global;
	char_frequency = frequency_global;
	pthread_barrier_destroy(&barrier);  //这里不需要，对吗？
	//free(zipped_char_global);
	free(arg_local);
	free(combine_global);
	//exit(0); //整这里应该是0？？？
}

//this function is for count the occurrence of chars in each thread
void *counter_chars(void *arg)
{
	struct arg_local *arg_local;
	arg_local = (struct arg_local *)arg;
	int max_local_size = input_chars_size_global * sizeof(struct zipped_char);
	struct zipped_char *zipped_char_local = malloc(max_local_size);
	//printf("zipped_char_local size in malloc:",sizeof(zipped_char_local)) ;
	//caculate the occurence of the characters,
	//and assigned value for the structure of zipped_char_local
	int start_index = arg_local->start_index;
	int zipped_count = 0;
	for (int j = start_index; j < (start_index+chars_block_size_global) - 1; j++) {
		if (j == start_index) {
			zipped_char_local[zipped_count].character = arg_local->input_chars[start_index];
			zipped_char_local[zipped_count].occurence = 1;
		}
		if (arg_local->input_chars[j] == arg_local->input_chars[j + 1]) {
			zipped_char_local[zipped_count].occurence++;
		}
		if (arg_local->input_chars[j] != arg_local->input_chars[j + 1]) {
			zipped_count++;
			zipped_char_local[zipped_count].character = arg_local->input_chars[j+1];
			zipped_char_local[zipped_count].occurence= 1;	
		}
	}
	//update the global value: 1,zipped_char_global; 2, frequency_global
	combine_global[arg_local->thIdD] = zipped_count + 1;
	pthread_barrier_wait(&barrier);
	int sum=0;
	for (int i = 0; i < arg_local->thIdD; i++)
	{
		sum = sum + combine_global[i];
	}
	pthread_mutex_lock(&mutex_lock01);
	for (int k = 0; k <= zipped_count; k++) {
		zipped_char_global[sum + k] = zipped_char_local[k];
		zipped_counter_global++;
	}
	for (int m = 0; m <= zipped_count; m++) {
		int letter_order = zipped_char_local[m].character - 'a';
		frequency_global[letter_order] = frequency_global[letter_order] + zipped_char_local[m].occurence;
	}
	pthread_mutex_unlock(&mutex_lock01);


	free(zipped_char_local);
	//pthread_barrier_wait(&barrier);
	pthread_exit((void *)zipped_char_local);
}