#ifndef STAGE_LIB_H
#define STAGE_LIB_H

#pragma once
#include "Queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

struct handler;

typedef void* (*function)(void* data, void* handler);

typedef struct stage{
    function func;
    pthread_t thread;
    Queue* sourseQu;
    Queue* destQu;
    bool is_active;
    struct stage* next;
    void* params;
    int id;
}stage;

struct stage_params;

typedef struct stage_handler{
    stage * head;
    struct stage_params* params_head;
    bool PIPELINE_ON;

}stage_handler;

typedef struct stage_params{
    stage* my_stage;
    bool* PIPELINE_ON;
    struct stage_params *next;
}stage_params;


typedef struct stage_t{

    void* (*create_handler)(void* pipe_header);
    void (*free_handler)(void* my_handler);
    void* (*create_stage)(Queue* sourse, Queue* dest, function func, void* params ,stage* next);
    void (*free_stage)(void* my_stage);
 /*   void (*free_stages)(void* my_stage);
    void (*free_stages_and_queues)(void* my_stage);*/
    void (*start_pipe)(void* my_handler, int stages_number, int is_pipe);
    void (*stop_pipe)(void* my_handler);
    void* (*main_stage_thread)(void* arg);

}stage_t;


#endif // STAGE_LIB_H
