#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h> 
#include <unistd.h>

#include "inventory.h"

#define TRANSACTIONS 10000  // to assign the number of transactions 
#define REFUND_VAL 7    // to get a 10% chance for returning a product 

// array instances 
product_t inventory[NUM_PRODUCTS];
user_t users[USER_COUNT];

// thread function for running transactions
void* run_transactions(void* arg){

    size_t userID=*(size_t*)arg;    // setting user id

    // for loop to run all the transactions
    for (size_t i = 0; i < TRANSACTIONS; i++){
        
        int random_prod = rand()%NUM_PRODUCTS;   //randomly getting product to buy
        int random_qty = rand()%100;    // randomly getting quantity to buy

        // checking if the product and quantity is valid in the inventory
        if (checkInventoryCount(random_prod, random_qty)){
            // buying product
            buy(random_prod, userID, random_qty);

            int rand_refund = rand()%10;    // there is a 10% chance that user will return their bought product

            // refunding the product
            if (rand_refund == REFUND_VAL){
                usleep(100);
                refund(random_prod, userID, random_qty);
            }
        }
        // if product quantity is not in inventory
        else{
            usleep(200);
            restock(random_prod);   // restocking the inventory

            // checking if the product and quantity is valid in the inventory
            if (checkInventoryCount(random_prod, random_qty)){
                // buying product 
                buy(random_prod, userID, random_qty);

                int rand_refund = rand()%10;    // there is a 10% chance that user will return their bought product
                
                // refunding the product 
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

// function that concurrently audits the revenue to ensure the revenue in the inventory matches the revenue from the users
void* audit(void* arg){

    // infinite loop to concurrently audit
    while(1){

        //initialize 2D array of total revenue for users products
        int total_revenue[USER_COUNT][NUM_PRODUCTS] = {{0}};

        //lock all products
        for (int j = 0; j < NUM_PRODUCTS; j++){
            pthread_mutex_lock(&inventory[j].lock);
        }

        //lock all users
        for (size_t i = 0; i < USER_COUNT; i++){
            pthread_mutex_lock(&users[i].user_lock);
        }

        //go through the whole array to get the revenue from amount bought by users
        for (size_t i = 0; i < USER_COUNT; i++){
            for (int j = 0; j < NUM_PRODUCTS; j++){
                //pthread_mutex_lock(&inventory[j].lock);   // for demo purpose
                //pthread_mutex_lock(&users[i].user_lock);  // for demo purpose
                total_revenue[i][j] += ((users[i].productsBought[j] * inventory[j].productCost) - (users[i].returnedProduct[j] * inventory[j].productCost));
                //pthread_mutex_unlock(&inventory[j].lock); // for demo purpose
                //pthread_mutex_unlock(&users[i].user_lock); // for demo purpose
            }
        }

        //adding the revenues together to get the total revenue
        int total = 0;

        for (size_t i = 0; i < USER_COUNT; i++){
            for (int j = 0; j < NUM_PRODUCTS; j++){
                total += total_revenue[i][j];
            }
        }

        //get the total revenue we calculated in the inventory method
        int checkTotal = inventoryOutputFile();

        //unlock all products
        for (int j = 0; j < NUM_PRODUCTS; j++){
                pthread_mutex_unlock(&inventory[j].lock);
        }

        //unlock all users
        for (size_t i = 0; i < USER_COUNT; i++){
            pthread_mutex_unlock(&users[i].user_lock);
        }

        //print "Correct" if the 2 totals are equal. "Incorrect" and Exit if they do not match
        if(checkTotal==total){
            printf("Audit Correct. The total revenue is %d\n", total);
        }
        else{
            printf("The Audit is Incorrect. Audit revenue: %d Inventory revenue: %d\n", total, checkTotal);
            exit(2);
        }

        //sleep consistently for 10000ms
        usleep(10000);
    }   
}

// main function
int main(int argc, char** argv){
    srand(time(NULL)); // seed to ensure randomization for each sequence

    inventory_init(); // initialize inventory

    pthread_t threads[USER_COUNT]; // user threads
    size_t user_id[USER_COUNT]; // array to store user id

    pthread_t threads_audit; // thread for auditing

    
    // create threads for each user 
    for (size_t i = 0; i < USER_COUNT; i++) {
        user_id[i] = i; // generate id for each user
        // create threads and check if thread creation is successful
        if (pthread_create(&threads[i], NULL, user_init, &user_id[i])) {
            perror("Thread creation not successful.\n");
            exit(2);
        }
    }

    // wait for each thread to finish
    for (size_t i = 0; i < USER_COUNT; i++) {
        // join threads and check for errors 
        if (pthread_join(threads[i], NULL)) {
            perror("Pthread join failed.\n");
            exit(2);
        }
    }

    pthread_t threads_running[USER_COUNT]; // threads for transactions

    // create threads and pass user id
    for (size_t i = 0; i < USER_COUNT; i++) {
        user_id[i] = i;
        // check if threads have been created successfully
        if (pthread_create(&threads_running[i], NULL, run_transactions, &user_id[i])) {
            perror("Thread creation not successful.\n");
            exit(2);
        }
    }

    // check if the audit thread has been created successfully
    if (pthread_create(&threads_audit, NULL, audit, NULL)) {
        perror("Thread creation not successful.\n");
        exit(2);
    }
    

    // wait for each thread to finish
    for (size_t i = 0; i < USER_COUNT; i++) {
        // join threads and check for errors 
        if (pthread_join(threads_running[i], NULL)) {
            perror("Pthread join failed.\n");
            exit(2);
        }
    }

    return 0;
}
