#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h> 
#include <unistd.h>

#include "inventory.h"

#define TRANSACTIONS 10000
#define REFUND_VAL 7

product_t inventory[NUM_PRODUCTS];
user_t users[USER_COUNT];

void* run_transactions(void* arg){

    size_t userID=*(size_t*)arg;

    for (size_t i = 0; i < TRANSACTIONS; i++){
        
        int random_prod = rand()%NUM_PRODUCTS;
        int random_qty = rand()%100;
        if (checkInventoryCount(random_prod, random_qty)){
            buy(random_prod, userID, random_qty);
            int rand_refund = rand()%10;
            if (rand_refund == REFUND_VAL){
                usleep(100);
                refund(random_prod, userID, random_qty);
            }
        }
        else{
            usleep(200);
            restock(random_prod);
            if (checkInventoryCount(random_prod, random_qty)){
                buy(random_prod, userID, random_qty);
                int rand_refund = rand()%10;
                if (rand_refund == REFUND_VAL){
                    usleep(100);
                    refund(random_prod, userID, random_qty);
                }
            }
            
        }
        
        usleep(100);
    }
    return NULL;
}

void* audit(void* arg){
    while(1){
     
        int total_revenue[USER_COUNT][NUM_PRODUCTS] = {{0}};

        for (int j = 0; j < NUM_PRODUCTS; j++){
            pthread_mutex_lock(&inventory[j].lock);
        }

        for (size_t i = 0; i < USER_COUNT; i++){
            pthread_mutex_lock(&users[i].user_lock);
        }

        for (size_t i = 0; i < USER_COUNT; i++){
            for (int j = 0; j < NUM_PRODUCTS; j++){
                //pthread_mutex_lock(&inventory[j].lock);
                //pthread_mutex_lock(&users[i].user_lock);
                total_revenue[i][j] += ((users[i].productsBought[j] * inventory[j].productCost) - (users[i].returnedProduct[j] * inventory[j].productCost));
                //pthread_mutex_unlock(&inventory[j].lock);
                //pthread_mutex_unlock(&users[i].user_lock);
            }
        }
        
        int total = 0;
        for (size_t i = 0; i < USER_COUNT; i++){
            for (int j = 0; j < NUM_PRODUCTS; j++){
                total += total_revenue[i][j];
            }
        }
        
        int checkTotal = inventoryOutputFile();

        for (int j = 0; j < NUM_PRODUCTS; j++){
                pthread_mutex_unlock(&inventory[j].lock);
        }

        for (size_t i = 0; i < USER_COUNT; i++){
            pthread_mutex_unlock(&users[i].user_lock);
        }
        
        if(checkTotal==total){
            printf("Audit Correct. The total revenue is %d\n", total);
        }
        else{
            printf("The Audit is Incorrect. Audit revenue: %d Inventory revenue: %d\n", total, checkTotal);
            exit(2);
        }
        
        usleep(10000);

    }
    
}

int main(int argc, char** argv){
    srand(time(NULL));

    inventory_init();

    pthread_t threads[USER_COUNT];
    size_t user_id[USER_COUNT];

    pthread_t threads_audit;

    

    for (size_t i = 0; i < USER_COUNT; i++) {
        user_id[i] = i;
        if (pthread_create(&threads[i], NULL, user_init, &user_id[i])) {
            perror("Thread creation not successful.\n");
            exit(2);
        }
    }

    // wait for each thread to finish
    for (size_t i = 0; i < USER_COUNT; i++) {
        if (pthread_join(threads[i], NULL)) {
            perror("Pthread join failed.\n");
            exit(2);
        }
    }

    pthread_t threads_running[USER_COUNT];

    for (size_t i = 0; i < USER_COUNT; i++) {
        user_id[i] = i;
        if (pthread_create(&threads_running[i], NULL, run_transactions, &user_id[i])) {
            perror("Thread creation not successful.\n");
            exit(2);
        }
    }

    if (pthread_create(&threads_audit, NULL, audit, NULL)) {
        perror("Thread creation not successful.\n");
        exit(2);
    }
    

    // wait for each thread to finish
    for (size_t i = 0; i < USER_COUNT; i++) {
        if (pthread_join(threads_running[i], NULL)) {
            perror("Pthread join failed.\n");
            exit(2);
        }
    }

    return 0;
}