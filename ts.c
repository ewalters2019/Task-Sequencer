/*
 * 
 * Compile: gcc -lpthread input.c -o output or gcc -pthread input.c -o output
 * 
 * TODO:
 * 
 * 1. Design FIFO api
 * 2. Add other functions, lol
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


///////////////////////////////////////////////////////////////////////////////


void ts_wait(){
    
    while(t_interupt){ }
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

        if((target == 0) && (t_index == 1)){ // only one task running
        
            pthread_cancel(t_list[target]->id);

            free(t_list[target]);
                
            free(t_list);
            
            t_list == NULL;
        
            t_index--;

        
        }else if(target == 0){ // del 1st task, but > 1 task running
        
            struct ts_task** temp = (struct ts_task**)calloc((t_index + 1), sizeof(struct ts_task*));
        
            memcpy(temp, (t_list + 1), (sizeof(struct ts_task*) * (t_index)));
        
            temp[t_index] = NULL;
   
        
            pthread_cancel(t_list[target]->id);

            free(t_list[target]);

            t_list = temp; 

            t_index--;


        }else if(target == (t_index - 1)){ // del last task
        

            t_list[target] = NULL;
            
            free(t_list[(t_index + 1)]);
           
            t_index--;

    
        }else{ // del task that is somewhere besides start or end

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

  
    sem_init(&task->lock,0,0);  


    pthread_create(&task->id, NULL, (void*)routine, (void*)task);
    
    pthread_detach(task->id);
    
    
    t_index++;
    

    ts_unpause();
    
}


void ts_write(int output, char* buffer){
    
    write(output, buffer, strlen(buffer));
}


void ts_read(int input){
    
    char buffer[1024];

    read(input, buffer, sizeof(buffer)); 
 
}


/////////////////////////////// EXAMPLE TASKS /////////////////////////////////


void task4(void* args){
    
    struct ts_task* task = args;
    
 	    
 	/* initialization code goes here */    
 	
 	
	while(1){
          
    	sem_wait(&task->lock);       
        
        ts_wait();

        t_current = task->index;
            
			
			printf("Running task: 4\n");  	
			
			/*
			  DO WORK HERE 
			  
			  Doesn't have to be inside a loop if only 
			  need it for 1 call, but the work doing done 
			  needs to be right here as the lock design 
			  requires it!!!
			
			*/

        

		sem_post(&ts_lock);       
    }
    
}

void task3(void* args){
    
    struct ts_task* task = args;
    
 	    
 	/* initialization code goes here */    
 	
 	
	while(1){
          
    	sem_wait(&task->lock);       
        
        ts_wait();

        t_current = task->index;
            
			
			printf("Running task: 3\n");  	
			
			/*
			  DO WORK HERE 
			  
			  Doesn't have to be inside a loop if only 
			  need it for 1 call, but the work doing done 
			  needs to be right here as the lock design 
			  requires it!!!
			
			*/

        
        sem_post(&ts_lock);       
            
    }
    
}

void task2(void* args){
    
    struct ts_task* task = args;
    
 	
 	/* initialization code goes here */ 
 	
 	  
	while(1){
          
    	sem_wait(&task->lock);       
        
        ts_wait();

        t_current = task->index;
            
			
			printf("Running task: 2\n");  	
			
			/*
			  DO WORK HERE 
			  
			  Doesn't have to be inside a loop if only 
			  need it for 1 call, but the work doing done 
			  needs to be right here as the lock design 
			  requires it!!!
			
			*/

      	
      	sem_post(&ts_lock);       
            
    }
    
}

void task1(void* args){
    
    struct ts_task* task = args;
    
    
    /* initialization goes code here */    
    
 	
	while(1){
          
    	sem_wait(&task->lock);       
        
        ts_wait();

        t_current = task->index;
            
			
			printf("Running task: 1\n");  	
			
			/*
			  DO WORK HERE 
			  
			  Doesn't have to be inside a loop if only 
			  need it for 1 call, but the work doing done 
			  needs to be right here as the lock design 
			  requires it!!!
			
			*/

        
		sem_post(&ts_lock);       
            
    }
    
}


///////////////////////////////////////////////////////////////////////////////


void interface(void* args){
    
	struct ts_task* task = args;
    
 	while(1){
          
    	sem_wait(&task->lock);       
        
        ts_wait();

        t_current = task->index;
            
			printf("\n\nRunning interface\n");  	
			
			//ts_read(task->input);
        
        	/*
        	
        	
        	
        	*/
        
        sem_post(&ts_lock);       
            
    }
    
}


///////////////////////////////////////////////////////////////////////////////


void ts_main(){
    
    
    while(1){ 

        if(t_list != NULL){

            for(int target = 0; t_list[target] != NULL; target++){

                ts_wait();

                sem_wait(&ts_lock);       

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
        
    ts_add_task("interface", &interface, 1, 1);
    
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

