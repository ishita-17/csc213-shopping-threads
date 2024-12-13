#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h> 

#include "inventory.h"

// function to initialize inventory
void inventory_init() {
    int check;
    // iterate through products
    for (size_t i = 0; i < NUM_PRODUCTS; i++) {
        inventory[i].productID=i; // initialize product ID
        inventory[i].inventoryCount=100; // set initial inventory count to 100
        inventory[i].countSold=0; // set initial products sold to 0
        check = pthread_mutex_init(&(inventory[i].lock),NULL);  // initialize each lock separately
        // check for errors
        if (check != 0){
            perror("Error initializing lock.\n");
            exit(2);
        }

    }
    // assign product name
    inventory[0].productName="Pen";
    inventory[1].productName="Pencil";
    inventory[2].productName="Eraser";
    inventory[3].productName="Sharpener";
    inventory[4].productName="Notebook";
    inventory[5].productName="Ruler";

    // assign product cost
    inventory[0].productCost=5;
    inventory[1].productCost=4;
    inventory[2].productCost=5;
    inventory[3].productCost=7;
    inventory[4].productCost=10;
    inventory[5].productCost=4;

}

// function to initialize user
void* user_init(void* arg){
    int i = *(int*)arg; // retrieve user ID from thread creation
    users[i].userID=(size_t)i; // assign user id
    memset(&users[i].productsBought, 0, sizeof(int)*NUM_PRODUCTS); // initialize product bought array
    memset(&users[i].returnedProduct, 0, sizeof(int)*NUM_PRODUCTS); // initialize product returned array
    
    int check = pthread_mutex_init(&(users[i].user_lock),NULL);  // initialize each lock separately
    // check for errors
    if (check != 0){
        perror("Error initializing lock.\n");
        exit(2);
    }
    return NULL;
}

// function to check the inventory count for a specific product
bool checkInventoryCount(size_t productID,int quantity){
    pthread_mutex_lock(&inventory[productID].lock); // lock the product
    // check if the quantity is enough
    bool val = (inventory[productID].inventoryCount) >= quantity;
    pthread_mutex_unlock(&inventory[productID].lock); // unlock
    return val;
}

// function to buy products
void buy(size_t productID,size_t userID,int quantity){
    pthread_mutex_lock(&inventory[productID].lock); // lock the product
    pthread_mutex_lock(&users[userID].user_lock); // lock the user
    users[userID].productsBought[productID]+= quantity; // increment user product bought array by quantity
    inventory[productID].inventoryCount-= quantity; // decrement inventory count by quantity for a specific product
    inventory[productID].countSold+= quantity; // increment inventory count by quantity for a specific product
    pthread_mutex_unlock(&users[userID].user_lock); // unlock user lock
    pthread_mutex_unlock(&inventory[productID].lock); // unlock product lock 
}

// function to refund products
void refund(size_t productID,size_t userID,int quantity){
    pthread_mutex_lock(&inventory[productID].lock); // lock the product
    pthread_mutex_lock(&users[userID].user_lock); // lock the user
    users[userID].returnedProduct[productID] += quantity; // increment user returned product by quantity
    inventory[productID].inventoryCount+= quantity; // increment inventory count by quantity for a specific product
    inventory[productID].countSold-= quantity; // decrement inventory count by quantity for a specific product
    pthread_mutex_unlock(&users[userID].user_lock); // unlock user lock
    pthread_mutex_unlock(&inventory[productID].lock); // unlock product lock
}

// function to restock inventory for a specific product
void restock(size_t productID){
    pthread_mutex_lock(&inventory[productID].lock); // lock the product
    inventory[productID].inventoryCount+=50; // increment inventory count by 50 for a specific product
    pthread_mutex_unlock(&inventory[productID].lock); // unlock the product
}

// function to write the total revenue to a file
int inventoryOutputFile(){
    FILE *f = fopen("output.txt", "w"); // open file 

    // check for errors
    if (f == NULL){
        perror("File failed to be opened.\n");
        exit(2);
    }
    int total_revenue[NUM_PRODUCTS]={0}; // initialize array 

    // iterate through products
    for (int j = 0; j < NUM_PRODUCTS; j++){
        //pthread_mutex_lock(&inventory[j].lock); // for demo purpose
        total_revenue[j] += (inventory[j].countSold * inventory[j].productCost); // calculate the total revenue by product sold and its cost
        //pthread_mutex_unlock(&inventory[j].lock); // for demo purpose
    }

    // output to file
    fprintf(f, "Product Name \tRevenue Earned\n");
    int total = 0; // set to 0 initially
    // iterate through products to write each product's total revenue
    for (size_t i = 0; i < NUM_PRODUCTS; i++){
        fprintf(f, "%s\t\t\t$%d\n", inventory[i].productName, total_revenue[i]);
        total += total_revenue[i];
    }
    fprintf(f, "Total Revenue Earned: %d\n", total); // write total revenue to file
    fclose(f); // close file
    return total; // return total to the audit function
}
