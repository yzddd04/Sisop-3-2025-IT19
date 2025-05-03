#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_ORDERS 100

typedef struct {
    char name[100];
    char address[200];
    char type[20];
    int delivered;
    char delivered_by[50];
} Order;

typedef struct {
    Order orders[MAX_ORDERS];
    int count;
} SharedMemory;

void log_delivery(const char* agent, const char* name, const char* address) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    
    FILE *log_file = fopen("delivery.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    
    fprintf(log_file, "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] Reguler package delivered to %s in %s\n",
            local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
            local->tm_hour, local->tm_min, local->tm_sec,
            agent, name, address);
    fclose(log_file);
}

void deliver_order(const char* name, const char* user) {
    int shmid = shmget(1234, sizeof(SharedMemory), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        return;
    }
    
    SharedMemory *shared_mem = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shared_mem == (void*)-1) {
        perror("shmat failed");
        return;
    }
    
    for (int i = 0; i < shared_mem->count; i++) {
        if (strcmp(shared_mem->orders[i].name, name) == 0 && 
            strcmp(shared_mem->orders[i].type, "Reguler") == 0 &&
            shared_mem->orders[i].delivered == 0) {
            
            // Simulate delivery
            sleep(1 + rand() % 3);
            
            shared_mem->orders[i].delivered = 1;
            strcpy(shared_mem->orders[i].delivered_by, user);
            
            log_delivery(user, name, shared_mem->orders[i].address);
            printf("Order for %s has been delivered by %s\n", name, user);
            
            shmdt(shared_mem);
            return;
        }
    }
    
    printf("No pending Reguler order found for %s\n", name);
    shmdt(shared_mem);
}

void check_status(const char* name) {
    int shmid = shmget(1234, sizeof(SharedMemory), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        return;
    }
    
    SharedMemory *shared_mem = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shared_mem == (void*)-1) {
        perror("shmat failed");
        return;
    }
    
    for (int i = 0; i < shared_mem->count; i++) {
        if (strcmp(shared_mem->orders[i].name, name) == 0) {
            if (shared_mem->orders[i].delivered) {
                printf("Status for %s: Delivered by Agent %s\n", name, shared_mem->orders[i].delivered_by);
            } else {
                printf("Status for %s: Pending\n", name);
            }
            
            shmdt(shared_mem);
            return;
        }
    }
    
    printf("No order found for %s\n", name);
    shmdt(shared_mem);
}

void list_orders() {
    int shmid = shmget(1234, sizeof(SharedMemory), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        return;
    }
    
    SharedMemory *shared_mem = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shared_mem == (void*)-1) {
        perror("shmat failed");
        return;
    }
    
    printf("\n=== ALL ORDERS ===\n");
    for (int i = 0; i < shared_mem->count; i++) {
        printf("%d. %s - %s - %s - %s\n", 
               i+1, 
               shared_mem->orders[i].name,
               shared_mem->orders[i].address,
               shared_mem->orders[i].type,
               shared_mem->orders[i].delivered ? "Delivered" : "Pending");
    }
    printf("\n");
    
    shmdt(shared_mem);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("  ./dispatcher -deliver [Nama] [User]\n");
        printf("  ./dispatcher -status [Nama]\n");
        printf("  ./dispatcher -list\n");
        return 1;
    }

    if (strcmp(argv[1], "-deliver") == 0) {
        if (argc < 3) {
            printf("Please specify the name and user\n");
            return 1;
        }
        deliver_order(argv[2], argv[3]);
    } 
    else if (strcmp(argv[1], "-status") == 0) {
        if (argc < 3) {
            printf("Please specify the name\n");
            return 1;
        }
        check_status(argv[2]);
    } 
    else if (strcmp(argv[1], "-list") == 0) {
        list_orders();
    }
    else {
        printf("Invalid command\n");
        return 1;
    }

    return 0;
}