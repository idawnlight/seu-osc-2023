#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapreduce.h"
#include "queue.h"

pthread_t* mappers;
pthread_t* reducers;

Mapper mapper;
Reducer reducer;
Partitioner partitioner;
int num_partitioner;

queue_t*  map_queue;

typedef struct data_unit {
    char* data;
    struct data_unit* next;
} data_unit_t;

typedef struct data_block {
    char* identifier;
    data_unit_t* data_chain;
    struct data_block* next;
} data_block_t;

data_block_t** data_blocks;
pthread_mutex_t* data_block_mutexes;

typedef struct mr_map_feeder_args {
    int num_files;
} mr_map_feeder_args_t;

typedef struct mr_reducer_args {
    int partition_number;
} mr_reducer_args_t;

typedef struct reducer_args {
    char* key;
    Getter get_func;
    int partition_number;
} reducer_args_t;

void* MR_Map_Feeder(void* args) {
    mr_map_feeder_args_t* mfa = (mr_map_feeder_args_t*) args;
    int num_files = mfa->num_files;
    for (int i = 1; i <= num_files; i++) {
        char* file_name = malloc(sizeof(char) * 100);
        sprintf(file_name, "testcase/sample%d.txt", i);
        enqueue(map_queue, file_name);
        printf("Enqueued %s\n", file_name);
    }
    return NULL;
}

void* MR_Mapper() {
    char* file_name = malloc(sizeof(char) * 100);
    dequeue(map_queue, &file_name);
    while (file_name != NULL) {
        printf("Mapping %s\n", file_name);
        mapper(file_name);
        dequeue(map_queue, &file_name);
    }
    return NULL;
}

char* MR_GetNext(char* key, int partition_number)
{
    pthread_mutex_lock(&data_block_mutexes[partition_number]);
    data_block_t* data_block = data_blocks[partition_number];
    while (data_block != NULL) {
        if (strcmp(data_block->identifier, key) == 0) {
            data_unit_t* data_chain = data_block->data_chain;
            if (data_chain == NULL) {
                pthread_mutex_unlock(&data_block_mutexes[partition_number]);
                return NULL;
            }
            data_block->data_chain = data_block->data_chain->next;
            pthread_mutex_unlock(&data_block_mutexes[partition_number]);
            return data_chain->data;
        }
        data_block = data_block->next;
    }
    pthread_mutex_unlock(&data_block_mutexes[partition_number]);
    return NULL;
}

void* MR_Reducer(void* args) {
    mr_reducer_args_t* r_args = (mr_reducer_args_t*) args;
    int partition_number = r_args->partition_number;
    while (data_blocks[partition_number] != NULL) {
        char* key = malloc(sizeof(char) * strlen(data_blocks[partition_number]->identifier));
        strcpy(key, data_blocks[partition_number]->identifier);
        reducer(key, MR_GetNext, partition_number);
        data_block_t* temp = data_blocks[partition_number];
        data_blocks[partition_number] = data_blocks[partition_number]->next;
        free(temp);
    }
    return NULL;
}

void MR_Emit(char *key, char *value)
{
    unsigned long partition_number = partitioner(key, num_partitioner);
    // if (strcmp(key, "and") == 0) {
    //     printf("Emitting %s %s to partition %lu\n", key, value, partition_number);
    // }
    // printf("Emitting %s %s to partition %lu\n", key, value, partition_number);
    // printf("Wait to lock partition %lu\n", partition_number);
    pthread_mutex_lock(&data_block_mutexes[partition_number]);
    // printf("Locked partition %lu\n", partition_number);
    data_block_t* data_block = data_blocks[partition_number];
    if (data_block == NULL) {
        data_block = malloc(sizeof(data_block_t));
        // data_block->identifier = key;
        data_block->identifier = malloc(sizeof(char) * strlen(key));
        strcpy(data_block->identifier, key);
        data_block->data_chain = malloc(sizeof(data_unit_t));
        data_block->data_chain->data = value;
        data_block->next = NULL;
        data_blocks[partition_number] = data_block;
    } else {
        while (data_block != NULL) {
            if (strcmp(data_block->identifier, key) == 0) {
                data_unit_t* data_unit = data_block->data_chain;
                data_unit_t* new_data_unit = malloc(sizeof(data_unit_t));
                new_data_unit->data = malloc(sizeof(char) * strlen(value));
                strcpy(new_data_unit->data, value);
                new_data_unit->next = data_unit;
                data_block->data_chain = new_data_unit;
                pthread_mutex_unlock(&data_block_mutexes[partition_number]);
                return;
            }
            data_block = data_block->next;
        }

        data_block_t* new_data_block = malloc(sizeof(data_block_t));
        new_data_block->identifier = malloc(sizeof(char) * strlen(key));
        strcpy(new_data_block->identifier, key);
        new_data_block->data_chain = malloc(sizeof(data_unit_t));
        new_data_block->data_chain->data = value;
        new_data_block->next = data_blocks[partition_number];
        data_blocks[partition_number] = new_data_block;
    }
    // printf("Prepare to unlock partition %lu\n", partition_number);
    pthread_mutex_unlock(&data_block_mutexes[partition_number]);
    // printf("Unlocked partition %lu\n", partition_number);
}

unsigned long MR_DefaultHashPartition(char* key, int num_partitions)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

void MR_Run(int argc, char *argv[], 
        Mapper map, int num_mappers, 
        Reducer reduce, int num_reducers, 
        Partitioner partition)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <num_files>\n", argv[0]);
        exit(1);
    }

    int num_files = atoi(argv[1]);
    mapper = map;
    reducer = reduce;
    partitioner = partition;
    num_partitioner = num_reducers;

    // Create data blocks
    data_blocks = malloc(num_reducers * sizeof(data_block_t*));
    for (int i = 0; i < num_reducers; i++) {
        data_blocks[i] = NULL;
    }

    // Create map queue
    map_queue = malloc(sizeof(queue_t));
    initialize(map_queue);

    // Create map feeder thread
    pthread_t map_feeder;
    mr_map_feeder_args_t* mfa = malloc(sizeof(mr_map_feeder_args_t));
    mfa->num_files = num_files;
    pthread_create(&map_feeder, NULL, MR_Map_Feeder, mfa);

    // Initialize data block mutexes
    data_block_mutexes = malloc(num_reducers * sizeof(pthread_mutex_t));
    for (int i = 0; i < num_reducers; i++) {
        pthread_mutex_init(&data_block_mutexes[i], NULL);
    }

    // Create mapper threads
    mappers = malloc(num_mappers * sizeof(pthread_t));
    for (int i = 0; i < num_mappers; i++) {
        pthread_create(&mappers[i], NULL, MR_Mapper, NULL);
    }

    for (int i = 0; i < num_mappers; i++) {
        sem_post(&map_queue->sem);
    }

    // Finish map feeder and mapper threads
    pthread_join(map_feeder, NULL);
    for (int i = 0; i < num_mappers; i++) {
        pthread_join(mappers[i], NULL);
    }

    // Create reducer threads
    reducers = malloc(num_reducers * sizeof(pthread_t));
    for (int i = 0; i < num_reducers; i++) {
        pthread_t reducer;
        mr_reducer_args_t* r_args = malloc(sizeof(mr_reducer_args_t));
        r_args->partition_number = i;
        pthread_create(&reducer, NULL, MR_Reducer, r_args);
        reducers[i] = reducer;
    }

    // for (int i = 0; i < 10; i++) {
    //     data_block_t* data_block = data_blocks[i];
    //     while (data_block != NULL) {
    //         printf("%s\n", data_block->identifier);
    //         data_block = data_block->next;
    //     }
    // }

    // Finish reducer threads
    for (int i = 0; i < num_reducers; i++) {
        pthread_join(reducers[i], NULL);
    }
}