#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <curl/curl.h>
#include <sys/stat.h>

#define MAX_ORDERS 100
#define MAX_NAME_LENGTH 100
#define MAX_ADDRESS_LENGTH 200
#define CSV_URL "https://drive.google.com/uc?export=download&id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9"
#define CSV_FILENAME "delivery_order.csv"

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

// Callback function for curl download
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Download CSV file using curl
int download_csv() {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return -1;
    }

    fp = fopen(CSV_FILENAME, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to create file\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, CSV_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
        fclose(fp);
        curl_easy_cleanup(curl);
        return -1;
    }

    fclose(fp);
    curl_easy_cleanup(curl);
    return 0;
}

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

void* agent_thread(void* arg) {
    char* agent_name = (char*)arg;
    int shmid = shmget(1234, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        pthread_exit(NULL);
    }
    
    SharedData* shared_data = (SharedData*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        pthread_exit(NULL);
    }
    
    while (1) {
        pthread_mutex_lock(&shared_data->mutex);
        
        for (int i = 0; i < shared_data->order_count; i++) {
            if (shared_data->orders[i].type == EXPRESS && 
                shared_data->orders[i].status == PENDING) {
                
                shared_data->orders[i].status = DELIVERED;
                strncpy(shared_data->orders[i].delivered_by, agent_name, 
                        sizeof(shared_data->orders[i].delivered_by) - 1);
                
                log_delivery(agent_name, shared_data->orders[i].name, 
                            shared_data->orders[i].address, EXPRESS);
                
                printf("AGENT %s: Delivered %s to %s\n", agent_name, 
                       shared_data->orders[i].name, shared_data->orders[i].address);
                
                pthread_mutex_unlock(&shared_data->mutex);
                sleep(1);
                break;
            }
        }
        
        pthread_mutex_unlock(&shared_data->mutex);
        sleep(1);
    }
    
    shmdt(shared_data);
    pthread_exit(NULL);
}

int main() {
    // Download CSV file automatically
    printf("Downloading delivery orders...\n");
    if (download_csv() != 0) {
        fprintf(stderr, "Failed to download CSV file\n");
        return 1;
    }
    printf("Download completed successfully\n");

    // Initialize shared memory
    int shmid = shmget(1234, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        return 1;
    }
    
    SharedData* shared_data = (SharedData*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        return 1;
    }
    
    // Initialize mutex
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_data->mutex, &attr);
    
    // Read CSV file
    FILE* file = fopen(CSV_FILENAME, "r");
    if (file == NULL) {
        perror("Failed to open CSV file");
        return 1;
    }
    
    char line[500];
    shared_data->order_count = 0;
    
    // Skip header
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) && shared_data->order_count < MAX_ORDERS) {
        char* name = strtok(line, ",");
        char* address = strtok(NULL, ",");
        char* type_str = strtok(NULL, "\n");
        
        if (name && address && type_str) {
            name = strtok(name, "\"");
            address = strtok(address, "\"");
            type_str = strtok(type_str, "\"");
            
            strncpy(shared_data->orders[shared_data->order_count].name, name, MAX_NAME_LENGTH - 1);
            strncpy(shared_data->orders[shared_data->order_count].address, address, MAX_ADDRESS_LENGTH - 1);
            
            if (strcasecmp(type_str, "Express") == 0) {
                shared_data->orders[shared_data->order_count].type = EXPRESS;
            } else {
                shared_data->orders[shared_data->order_count].type = REGULAR;
            }
            
            shared_data->orders[shared_data->order_count].status = PENDING;
            memset(shared_data->orders[shared_data->order_count].delivered_by, 0, 50);
            
            shared_data->order_count++;
        }
    }
    
    fclose(file);
    
    // Create agent threads
    pthread_t agent_a, agent_b, agent_c;
    pthread_create(&agent_a, NULL, agent_thread, "A");
    pthread_create(&agent_b, NULL, agent_thread, "B");
    pthread_create(&agent_c, NULL, agent_thread, "C");
    
    pthread_join(agent_a, NULL);
    pthread_join(agent_b, NULL);
    pthread_join(agent_c, NULL);
    
    shmdt(shared_data);
    return 0;
}