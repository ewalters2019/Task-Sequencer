/*
 * Code started on 11SEP19 by walters_w_eric@hotmail.com
 * 
 * Compile: gcc -lpthread input.c -o output or gcc -pthread input.c -o output
 * 
 * TODO:
 * 
 * 1. Design FIFO api
 * 2. Add "rake" functions
 * 3. Add signal handlers for cleanup
 * 4. Add cleanup functionality
 * 5. Complete documentation
 * 
 */


#include <stdio.h>      
#include <stdlib.h>     // malloc(), calloc(), realloc()
#include <unistd.h>     // sleep()
#include <string.h>  
#include <pthread.h>    
#include <semaphore.h>  
#include <fcntl.h>      // open()
#include <sys/stat.h>   // mkfifo()
#include <errno.h>     
#include <sys/types.h>  // primitive sys data types 

#include "cJSON.h"


int t_index; 
int t_current;
int t_interupt;

sem_t ts_lock;

struct ts_task{
          
        int index;
        
        char* name;
        
        int input;
        int output;
        
        sem_t lock;
        pthread_t id;
        
        void* storage;  
        
} **t_list;


struct t_params{
    
        struct ts_task* task;
        
        void (*routine)(void* storage);
};

struct t_storage{
 
        char* JSON;
};


///////////////////////////////////////////////////////////////////////////////


void ts_wait(){
    
    while(t_interupt){ }
}


void ts_task(void* args){
    
    
    struct t_params* ts = args;
 
    ts->task->storage = (struct t_storage*)calloc(1, sizeof(struct t_storage));

    
    while(1){
          
        sem_wait(&ts->task->lock);       
        
        ts_wait();

            t_current = ts->task->index;
            
            (*ts->routine)(ts->task->storage);
            
            //sleep(1);

            sem_post(&ts_lock);       
            
    }
    
}


int ts_get_fifo_out(struct ts_task* task){
    
    
    char* temp;
    
    temp = "./output/";    
    
    
    int size = (strlen((const char*)temp) + strlen((const char*)task->name));
    
    
    char* path = (char*)malloc(sizeof(char) * size);
    
    strncat(path, temp, (strlen(temp) + 1));    
    
    strncat(path, task->name, (strlen(task->name) + 1));       

    
    mkfifo(path, 0200);
    
    
    task->output = open(path, O_WRONLY | O_NONBLOCK);
}


void ts_get_fifo_in(struct ts_task* task){
    
    
    char* temp;
    
    temp = "./input/";    
    
    
    int size = (strlen((const char*)temp) + strlen((const char*)task->name));
    
    
    char* path = (char*)malloc(sizeof(char) * size);
    
    strncat(path, temp, (strlen(temp) + 1));    
    
    strncat(path, task->name, (strlen(task->name) + 1));       

    
    mkfifo(path, 0400);
        
    task->input = open(path, O_RDONLY | O_NONBLOCK);
}


char* ts_itoa(int num){
    
    char* buffer = (char*)calloc(8, sizeof(char)); //8 digits max length
    
    snprintf(buffer, 8, "%d", num);     
    
 
 return buffer;
}


void ts_unpause(){
        
    t_interupt = 0;
}


void ts_pause(){
 
    t_interupt = 1;
}


int ts_del_task(char* name){
  
    ts_pause();
    

    int target = 0;
    int found = 0;

    for(int j = 0; t_list[j] != NULL; j++){
        
        if(name == t_list[j]->name){ target = j; found = 1; break; }
        
    }
    
    
    if(found){   

        if((target == 0) && (t_index == 1)){
        
            pthread_cancel(t_list[target]->id);

            free(t_list[target]);
                
            free(t_list);
            
            t_list == NULL;
        
            t_index--;

        
        }else if(target == 0){
        
            struct ts_task** temp = (struct ts_task**)calloc((t_index + 1), sizeof(struct ts_task*));
        
            memcpy(temp, (t_list + 1), (sizeof(struct ts_task*) * (t_index)));
        
            temp[t_index] = NULL;
   
        
            pthread_cancel(t_list[target]->id);

            free(t_list[target]);

            t_list = temp; 

            t_index--;


        }else if(target == (t_index - 1)){
        

            t_list[target] = NULL;
            
            free(t_list[(t_index + 1)]);
           
            t_index--;

    
        }else{

            struct ts_task** temp = (struct ts_task**)calloc((t_index + 1), sizeof(struct ts_task*));

            memcpy(temp, t_list, (sizeof(struct ts_task*) * target));

            memcpy((temp + target), (t_list + (target + 1)), (sizeof(struct ts_task*) * (t_index - (target + 1))));
        
            temp[t_index] = NULL;
        
    
            pthread_cancel(t_list[target]->id);

    
            free(t_list[target]);

            t_list = temp;
        
            t_index--;
        }
   

        ts_unpause();

        
        if(target == t_current){ 
        
            int val = 0;
        
            sem_getvalue(&ts_lock,&val);
        
            if(val == 0){ sem_post(&ts_lock); }
        }
        
    }
    
}


void ts_add_task(char* name, void* routine, int input, int output){

    ts_pause();
    
    struct t_params* args;
    
    
    struct ts_task* task = (struct ts_task*)calloc(1, sizeof(struct ts_task));
    
    
    t_list = (struct ts_task**)realloc(t_list, (sizeof(struct ts_task*) * (t_index + 2)));

    t_list[t_index] = task;

    t_list[(t_index + 1)] = NULL;

    
    task->index = t_index;
    
    (name == NULL) ? (task->name = ts_itoa(t_index)) : (task->name = name);
    
    
    if(input){ ts_get_fifo_in(task); } 
    
    if(output){ ts_get_fifo_out(task); }

    
    //if(t_index > 0) { t_list[(t_index - 1)]->next = (void*)task; }
    
    //task->next = t_list[0];
    
    
    args = (struct t_params*)calloc(1, sizeof(struct t_params));

    args->task = task;
    args->routine = (void*)routine;
    
    sem_init(&task->lock,0,0);  

    pthread_create(&task->id, NULL, (void*)ts_task, (void*)args);
    
    pthread_detach(task->id);
    
    
    t_index++;
    

    ts_unpause();
    
}


/////////////////////////////// TASKS /////////////////////////////////////////



void dummy(void* store){
    
    struct t_storage* storage = store;
    
    storage->JSON = "DUMMY";
    
    printf("\nRunning task: %s\n\n", storage->JSON);  

}

void task1(void* storage){
     
    printf("Running task: 1\n\n");  

}

void task2(void* storage){
    
    printf("Running task: 2\n\n");

}

void task3(void* storage){
    
    printf("Running task: 3\n\n");

}


///////////////////////////////////////////////////////////////////////////////


void ts_write(){
    
    char buffer[1024];

    write(t_list[0]->output, buffer, strlen(buffer));
}


void ts_read(){
    
    char buffer[1024];

    read(t_list[0]->input, buffer, sizeof(buffer)); 
 
}



void ts_main(){
    
    ts_get_fifo_in(t_list[0]);
    ts_get_fifo_out(t_list[0]);

    
    while(1){ 

        if(t_list != NULL){

            for(int target = 0; t_list[target] != NULL; target++){

                ts_wait();

                sem_wait(&ts_lock);       

                /////////////////
                
                    ts_read();
                    
                /////////////////
                
                sem_post(&t_list[target]->lock);       
            }   
        }
    }
    
}


void ts_init(){
    
    t_list = NULL;
    t_index = 0;
    t_interupt = 0;
        
    sem_init(&ts_lock,0,1);  
        
    ts_add_task("dummy", &dummy, 0, 0);
    
    ts_add_task("task1", &task1, 0, 0);
    ts_add_task("task2", &task2, 0, 0);
    ts_add_task("task3", &task3, 0, 0);

}


///////////////////////////////////////////////////////////////////////////////


int main(){
    
  
    ts_init();
    
    
    ts_main();
    
    
 return 0;
}
