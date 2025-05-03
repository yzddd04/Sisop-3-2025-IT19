#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <curl/curl.h>

#define MAX_ORDERS 100
#define MAX_NAME_LENGTH 100
#define MAX_ADDRESS_LENGTH 200

typedef enum {
    PENDING,
    DELIVERED
} DeliveryStatus;

typedef enum {
    REGULAR,
    EXPRESS
} DeliveryType;

typedef struct {
    char name[MAX_NAME_LENGTH];
    char address[MAX_ADDRESS_LENGTH];
    DeliveryType type;
    DeliveryStatus status;
    char delivered_by[50];
} Order;

typedef struct {
    Order orders[MAX_ORDERS];
    int order_count;
    pthread_mutex_t mutex;
} SharedData;

void log_delivery(const char* agent_name, const char* name, const char* address, DeliveryType type) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    
    FILE *log_file = fopen("delivery.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    
    fprintf(log_file, "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] %s package delivered to %s in %s\n",
            local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
            local->tm_hour, local->tm_min, local->tm_sec,
            agent_name,
            type == EXPRESS ? "Express" : "Reguler",
            name, address);
    
    fclose(log_file);
}

void deliver_order(const char* agent_name, const char* customer_name) {
    int shmid = shmget(1234, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    
    SharedData* shared_data = (SharedData*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    pthread_mutex_lock(&shared_data->mutex);
    
    for (int i = 0; i < shared_data->order_count; i++) {
        if (strcmp(shared_data->orders[i].name, customer_name) == 0 && 
            shared_data->orders[i].type == REGULAR && 
            shared_data->orders[i].status == PENDING) {
            
            shared_data->orders[i].status = DELIVERED;
            strncpy(shared_data->orders[i].delivered_by, agent_name, 
                    sizeof(shared_data->orders[i].delivered_by) - 1);
            shared_data->orders[i].delivered_by[sizeof(shared_data->orders[i].delivered_by) - 1] = '\0';
            
            log_delivery(agent_name, shared_data->orders[i].name, 
                        shared_data->orders[i].address, REGULAR);
            
            printf("AGENT %s: Delivered %s to %s\n", agent_name, 
                   shared_data->orders[i].name, shared_data->orders[i].address);
            
            pthread_mutex_unlock(&shared_data->mutex);
            shmdt(shared_data);
            return;
        }
    }
    
    pthread_mutex_unlock(&shared_data->mutex);
    shmdt(shared_data);
    printf("No pending Regular order found for %s\n", customer_name);
}

void check_status(const char* customer_name) {
    int shmid = shmget(1234, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    
    SharedData* shared_data = (SharedData*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    pthread_mutex_lock(&shared_data->mutex);
    
    for (int i = 0; i < shared_data->order_count; i++) {
        if (strcmp(shared_data->orders[i].name, customer_name) == 0) {
            if (shared_data->orders[i].status == DELIVERED) {
                printf("Status for %s: Delivered by Agent %s\n", 
                       customer_name, shared_data->orders[i].delivered_by);
            } else {
                printf("Status for %s: Pending\n", customer_name);
            }
            
            pthread_mutex_unlock(&shared_data->mutex);
            shmdt(shared_data);
            return;
        }
    }
    
    pthread_mutex_unlock(&shared_data->mutex);
    shmdt(shared_data);
    printf("No order found for %s\n", customer_name);
}

void list_orders() {
    int shmid = shmget(1234, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    
    SharedData* shared_data = (SharedData*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    pthread_mutex_lock(&shared_data->mutex);
    
    printf("Order List:\n");
    printf("%-20s %-10s %-10s %-20s\n", "Name", "Type", "Status", "Delivered By");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < shared_data->order_count; i++) {
        printf("%-20s %-10s %-10s %-20s\n", 
               shared_data->orders[i].name,
               shared_data->orders[i].type == EXPRESS ? "Express" : "Regular",
               shared_data->orders[i].status == PENDING ? "Pending" : "Delivered",
               shared_data->orders[i].status == DELIVERED ? shared_data->orders[i].delivered_by : "-");
    }
    
    pthread_mutex_unlock(&shared_data->mutex);
    shmdt(shared_data);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("  ./dispatcher -deliver [Customer Name]\n");
        printf("  ./dispatcher -status [Customer Name]\n");
        printf("  ./dispatcher -list\n");
        return 1;
    }
    
    if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
        deliver_order(getenv("USER"), argv[2]);
    } 
    else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        check_status(argv[2]);
    }
    else if (strcmp(argv[1], "-list") == 0 && argc == 2) {
        list_orders();
    }
    else {
        printf("Invalid command or arguments\n");
        return 1;
    }
    
    return 0;
}
