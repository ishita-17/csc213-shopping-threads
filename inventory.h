#pragma once

#include <stdlib.h>

#define NUM_PRODUCTS 6
#define USER_COUNT 5

typedef struct product {
    size_t productID;
    char* productName;
    int productCost;
    int inventoryCount;
    int countSold;
    pthread_mutex_t lock;
} product_t;

typedef struct user {
    size_t userID;
    int productsBought[NUM_PRODUCTS];
    int returnedProduct[NUM_PRODUCTS];
    pthread_mutex_t user_lock;
} user_t;

extern product_t inventory[NUM_PRODUCTS];
extern user_t users[USER_COUNT];


void inventory_init();
void* user_init(void* arg);
bool checkInventoryCount(size_t productID,int quantity);
void buy(size_t productID,size_t userID,int quantity);
void refund(size_t productID,size_t userID,int quantity);
void restock(size_t productID);
int inventoryOutputFile();