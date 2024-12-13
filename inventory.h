#pragma once

#include <stdlib.h>
//Macro definition
#define NUM_PRODUCTS 6 //total number of products we have
#define USER_COUNT 5 //total number of users we have
//Struct for product
typedef struct product {
    size_t productID; 
    char* productName; 
    int productCost;
    int inventoryCount;
    int countSold;
    pthread_mutex_t lock;
} product_t;

//Struct for users
typedef struct user {
    size_t userID;
    int productsBought[NUM_PRODUCTS];
    int returnedProduct[NUM_PRODUCTS];
    pthread_mutex_t user_lock;
} user_t;

//array declarations
extern product_t inventory[NUM_PRODUCTS];
extern user_t users[USER_COUNT];

//function declarations
void inventory_init();
void* user_init(void* arg);
bool checkInventoryCount(size_t productID,int quantity);
void buy(size_t productID,size_t userID,int quantity);
void refund(size_t productID,size_t userID,int quantity);
void restock(size_t productID);
int inventoryOutputFile();
