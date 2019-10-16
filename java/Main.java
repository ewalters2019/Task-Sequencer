import java.util.concurrent.Semaphore;
import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Iterator;


public class Main{
    
    
    ////////////////////////////////////// GLOBALS ///////////////////////////////////////////	
	

	static class Task{
    
           int index;
        
           String name;
        
           int input;
           int output;
        
           Semaphore lock;

	   boolean alive;
        }

        static Semaphore g_lock;

	static List<Task> t_list;
	
	static int     t_index; 
	static int     t_current;
	static boolean t_interupt;
	

	////////////////////////////////////// INTERFACE ///////////////////////////////////////////	
	
    /* 	This interface is what allows us to pass our class/routine 
     * 	to the addTask(Routine routine) function and initialize its
     * 	members without knowing what it is.
    */	

	
	public interface Routine{
       
        public void init(Semaphore lock, String name);
    
        public void execute();    
    }

    
    ////////////////////////////////////// ROUTINES ///////////////////////////////////////////	
    

    public static class Feed extends Thread implements Routine{
        
        Semaphore lock;        
        String name;
        Boolean alive;        
        
        
        public void run(){
        
			
			while(alive){
				
				try{ this.lock.acquire(); } catch(Exception e){ System.out.println(e); } 	

				block();
					
					System.out.println("Task: "+this.name);
			
			        g_lock.release();           
			}                	
                             

        }        
        
	/////// Interface Methods /////////        
        
        public void execute(){

	    this.start();
	}
        
        public void init(Semaphore lock, String name){
            
	        this.lock = lock;
		this.name = name;
		this.alive = true;		
	}
		    
    
    } // end Feed
    
    
    ////////////////////////////////////// MAIN ///////////////////////////////////////////	
	
	public static void unpause(){
 
	    t_interupt = false;
        }
	
	public static void pause(){
 
	    t_interupt = true;
        }
    
	public static void block(){
    
   	    while(t_interupt){ }
	}

    
        public static void addTask(Routine routine, String name){
        
            pause();
        
            Semaphore lock = new Semaphore(1);
        
            try{ lock.acquire(); } catch(Exception e){ System.out.println(e); }         

            Task task = new Task();

	    task.lock = lock;
	    task.name = name;

            t_list.add(task);
        
	    routine.init(lock, name);
         
	    routine.execute();
        
            unpause();
        }

	public static void init(){
	
	    g_lock = new Semaphore(1);
		
            t_index    = 0;        
            t_current  = 0;
            t_interupt = false;
        
            t_list = new ArrayList<Task>();
        
		
	    addTask(new Feed(), "feed1");
	    addTask(new Feed(), "feed2");

	}    
    
    
        public static void main(String[] args){
    	
    	    init();
        
            while(true){ 
        
	        block();
				        	
                for (int i = 0; i < t_list.size(); i++){
                 
	            try{ g_lock.acquire(); } catch(Exception e){ System.out.println(e); }         
            	
            	    t_list.get(i).lock.release();
               }
  		
  	    } 

	}

}
