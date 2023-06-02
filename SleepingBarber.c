#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_CHAIRS 5

sem_t customers;  // Semaphore for customers
sem_t barber;     // Semaphore for barber
sem_t mutex;      // Semaphore for mutual exclusion
int num_waiting = 0; // Number of customers waiting

void *customer(void *arg) {
    int id = *((int *) arg);
    
    // Check if there are free chairs
    sem_wait(&mutex); // Acquire the mutex semaphore to protect shared data
    if (num_waiting == NUM_CHAIRS) { // If all chairs are occupied, customer leaves
        printf("Customer %d is leaving because there are no free chairs.\n", id);
        sem_post(&mutex); // Release the mutex semaphore
        free(arg); // Free the memory allocated for the customer ID
        return NULL; // Customer thread exits
    }
    num_waiting++; // Increment the number of customers waiting
    printf("Customer %d has arrived and is waiting.\n", id);
    sem_post(&mutex); // Release the mutex semaphore
    
    // Notify the barber
    sem_post(&barber); // Signal the barber that a customer has arrived
    
    // Wait for the barber to cut the hair
    sem_wait(&customers); // Wait for the barber to signal that the haircut is done
    
    printf("Customer %d is getting a haircut.\n", id);
    
    // Customer is done
    sem_wait(&mutex); // Acquire the mutex semaphore to protect shared data
    num_waiting--; // Decrement the number of customers waiting
    sem_post(&mutex); // Release the mutex semaphore
    
    free(arg); // Free the memory allocated for the customer ID
    
    return NULL;
}

void *barber_thread(void *arg) {
    while (1) {
        // Barber is sleeping
        
        printf("Barber is sleeping.\n");
        
        // Wait for a customer to arrive
        sem_wait(&barber); // Wait for a customer to signal arrival
        
        // Barber is awake
        printf("Barber is woke up and cutting hair.\n");
        
        // Cut the hair
        sleep(2);
        
        // Notify the customer that the haircut is done
        sem_post(&customers); // Signal the customer that the haircut is done
    }
    
    return NULL;
}

int main() {
    pthread_t barber_tid;
    pthread_t customer_tid[10];  // 10 customers
    
    // Initialize semaphores
    sem_init(&customers, 0, 0); // Initialize customers semaphore with 0 initial value
    sem_init(&barber, 0, 0); // Initialize barber semaphore with 0 initial value
    sem_init(&mutex, 0, 1); // Initialize mutex semaphore with 1 initial value
    
    // Create the barber thread
    pthread_create(&barber_tid, NULL, barber_thread, NULL);
    
    int i;
    for (i = 0; i < 10; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        // Create customer threads
        pthread_create(&customer_tid[i], NULL, customer, id);
        sleep(1); // Delay between customer arrivals
    }
    
    // Wait for the barber thread to finish
    pthread_join(barber_tid, NULL);
    for (i = 0; i < 10; i++) {
        // Wait for customer threads to finish
        pthread_join(customer_tid[i], NULL);
    }
    
    // Destroy semaphores
    sem_destroy(&customers);
    sem_destroy(&barber);
    sem_destroy(&mutex);
    
    return 0;
}
