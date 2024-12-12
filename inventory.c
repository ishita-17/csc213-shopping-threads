#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h> 

#include "inventory.h"

void inventory_init() {
    int check;
    for (size_t i = 0; i < NUM_PRODUCTS; i++) {
        inventory[i].productID=i;
        inventory[i].inventoryCount=100;
        inventory[i].countSold=0;
        check = pthread_mutex_init(&(inventory[i].lock),NULL);  // initialize each lock separately
        if (check != 0){
            perror("Error initializing lock.\n");
            exit(2);
        }

    }
    inventory[0].productName="Pen";
    inventory[1].productName="Pencil";
    inventory[2].productName="Eraser";
    inventory[3].productName="Sharpener";
    inventory[4].productName="Notebook";
    inventory[5].productName="Ruler";

    inventory[0].productCost=5;
    inventory[1].productCost=4;
    inventory[2].productCost=5;
    inventory[3].productCost=7;
    inventory[4].productCost=10;
    inventory[5].productCost=4;

}

void* user_init(void* arg){
    int i = *(int*)arg;
    users[i].userID=(size_t)i;
    memset(&users[i].productsBought, 0, sizeof(int)*NUM_PRODUCTS);
    memset(&users[i].returnedProduct, 0, sizeof(int)*NUM_PRODUCTS);
    
    int check = pthread_mutex_init(&(users[i].user_lock),NULL);  // initialize each lock separately
    if (check != 0){
        perror("Error initializing lock.\n");
        exit(2);
    }
    return NULL;
}

bool checkInventoryCount(size_t productID,int quantity){
    pthread_mutex_lock(&inventory[productID].lock);
    bool val = (inventory[productID].inventoryCount) >= quantity;
    pthread_mutex_unlock(&inventory[productID].lock);
    return val;
}

void buy(size_t productID,size_t userID,int quantity){
    pthread_mutex_lock(&inventory[productID].lock);
    pthread_mutex_lock(&users[userID].user_lock);
    users[userID].productsBought[productID]+= quantity;
    inventory[productID].inventoryCount-= quantity;
    inventory[productID].countSold+= quantity;
    pthread_mutex_unlock(&users[userID].user_lock);
    pthread_mutex_unlock(&inventory[productID].lock);
}

void refund(size_t productID,size_t userID,int quantity){
    pthread_mutex_lock(&inventory[productID].lock);
    pthread_mutex_lock(&users[userID].user_lock);
    users[userID].returnedProduct[productID] += quantity;
    inventory[productID].inventoryCount+= quantity;
    inventory[productID].countSold-= quantity;
    pthread_mutex_unlock(&users[userID].user_lock);
    pthread_mutex_unlock(&inventory[productID].lock);
}

void restock(size_t productID){
    pthread_mutex_lock(&inventory[productID].lock);
    inventory[productID].inventoryCount+=50;
    pthread_mutex_unlock(&inventory[productID].lock);
}

int inventoryOutputFile(){
    FILE *f = fopen("output.txt", "w");
    if (f == NULL){
        perror("File failed to be opened.\n");
        exit(2);
    }
    int total_revenue[NUM_PRODUCTS]={0};

    for (int j = 0; j < NUM_PRODUCTS; j++){
        //pthread_mutex_lock(&inventory[j].lock);
        total_revenue[j] += (inventory[j].countSold * inventory[j].productCost);
        //pthread_mutex_unlock(&inventory[j].lock);
    }

    fprintf(f, "Product Name \tRevenue Earned\n");
    int total = 0;
    for (size_t i = 0; i < NUM_PRODUCTS; i++){
        fprintf(f, "%s\t\t\t$%d\n", inventory[i].productName, total_revenue[i]);
        total += total_revenue[i];
    }
    fprintf(f, "Total Revenue Earned: %d\n", total);
    fclose(f);
    return total;
}