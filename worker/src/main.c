#include <stdio.h>
#include "worker.h"

typedef struct{
    int number;
    char ch;
    char* str;
}data_struct;


void callback1 (void* _Context, uint64_t _MonTime){
    data_struct* ctx = (data_struct*)_Context;
    printf("callback 1, number: %d\n", ctx->number);
    ctx->number += 1;
}

int main(){

    worker new_worker;

    worker_init(&new_worker);

    data_struct data1;
    data1.number = 1;
    data1.ch = 'a';


    worker_task* task1 = worker_createTask(&new_worker, &data1, callback1);

    while(1){
        worker_work(&new_worker, 0);
    }

    return 0;
}