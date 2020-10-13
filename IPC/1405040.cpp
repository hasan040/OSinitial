#include<iostream>
#include<cstdio>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include<cstring>
#include<time.h>
#include<chrono>
#include<thread>

using namespace std;

#define total_cycles 10
#define total_servicemen 3
#define payment_room_capacity 2

void * cycleman_task(void * arg);

pthread_mutex_t mutex;

pthread_mutex_t mutex_handler[total_servicemen];
pthread_mutex_t mutex_bill_handler[payment_room_capacity];

pthread_mutex_t mutex_door_opener;

sem_t service_handler;



sem_t sem_service_handler[total_servicemen];

sem_t sem_pay_cash;

sem_t service_capacity;





bool block_the_entry = false;



int main(){

    int res;

    sem_init(&service_handler,0,1);

    sem_init(&service_capacity,0,total_servicemen);  //for the room

    sem_init(&sem_pay_cash,0,payment_room_capacity);

    pthread_mutex_init(&mutex,NULL);

    pthread_mutex_init(&mutex_door_opener,NULL);


    for(int i=0;i<total_servicemen;i++){
        sem_init(&sem_service_handler[i],0,1);
    }

    for(int i=0;i<total_servicemen;i++){
        pthread_mutex_init(&mutex_handler[i],NULL);
    }

    for(int i=0;i<payment_room_capacity;i++){
        pthread_mutex_init(&mutex_bill_handler[i],NULL);
    }

    pthread_t cycleman[total_cycles];





    for(int i=0;i<total_cycles;i++){
        char * cycleman_id = new char[3];
        strcpy(cycleman_id,to_string(i+1).c_str());

        res = pthread_create(&cycleman[i],NULL,cycleman_task,(void *)cycleman_id);

        if(res != 0){
            perror("Thread creation failed!");
            exit(EXIT_FAILURE);

        }
    }

    for(int i=0;i<total_cycles;i++){
        void * ret_val;
        res = pthread_join(cycleman[i],&ret_val);
        if(res != 0){
            perror("Thread join failed!");
            exit(EXIT_FAILURE);
        }


    }

    sem_destroy(&service_handler);


    for(int i=0;i<total_servicemen;i++){
        sem_destroy(&sem_service_handler[i]);
    }

    sem_destroy(&sem_pay_cash);

    sem_destroy(&service_capacity);//for the room capacity

    pthread_mutex_destroy(&mutex);




    for(int i=0;i<total_servicemen;i++){
        pthread_mutex_destroy(&mutex_handler[i]);
    }


    for(int i=0;i<payment_room_capacity;i++){
        pthread_mutex_destroy(&mutex_bill_handler[i]);
    }


    pthread_mutex_destroy(&mutex_door_opener);








    return 0;

}

void * cycleman_task(void * arg){

   int random_num;


    while(true){

        sem_wait(&service_capacity);

        if(block_the_entry == false){

            int service_counter = 0;

            while(service_counter < total_servicemen){

                sem_wait(&sem_service_handler[service_counter]);
                pthread_mutex_lock(&mutex_handler[service_counter]);

                printf("%s has started taking service from serviceman %d \n",(char *)arg,(service_counter + 1));
                random_num = (rand() % 3) + 1;

                //sleep(random_num);

                this_thread::sleep_for(chrono::milliseconds(random_num * 100));


                printf("%s finished service from serviceman %d\n",(char *)arg,(service_counter + 1));
                service_counter++;
                pthread_mutex_unlock(&mutex_handler[service_counter-1]);
                sem_post(&sem_service_handler[service_counter-1]);


            }

            sem_post(&service_capacity);

            break;

        }




    }

    while(true){

        int sem_value;

        sem_wait(&sem_pay_cash);

        sem_getvalue(&sem_pay_cash,&sem_value);



        pthread_mutex_lock(&mutex_bill_handler[sem_value]);

        printf("%s is paying the bill from counter %d\n",(char *)arg,(sem_value + 1));
        random_num = (rand() % 3) + 1;

        //sleep(random_num);
        this_thread::sleep_for(chrono::milliseconds(random_num * 100));



        printf("%s has finished paying the bill from counter %d\n",(char *)arg,(sem_value +1));

        block_the_entry = true;

        pthread_mutex_unlock(&mutex_bill_handler[sem_value]);

        sem_post(&sem_pay_cash);
        break;

    }



    while(true){
        bool get_the_empty_status = true;

        for(int i=0;i<total_servicemen;i++){

            int sem_value_status;
            sem_getvalue(&sem_service_handler[i],&sem_value_status);

            if(sem_value_status == 0){
                get_the_empty_status = false;
                break;
            }

        }

        if(get_the_empty_status == true){
            printf("%s has departed \n",(char *) arg);

            break;
        }



    }

    block_the_entry = false;








    pthread_exit(arg);


}



