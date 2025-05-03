#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <pthread.h>
#include <curl/curl.h>

#define MAX_ORDERS 100
#define CSV_URL "https://drive.google.com/uc?export=download&id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9"
#define CSV_FILENAME "delivery_order.csv"

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

// Callback function for curl to write data to file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// Function to download the CSV file
int download_csv() {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(CSV_FILENAME, "wb");
        if (!fp) {
            perror("Failed to create CSV file");
            return -1;
        }
        
        curl_easy_setopt(curl, CURLOPT_URL, CSV_URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }
        
        fclose(fp);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize curl\n");
        return -1;
    }
    
    return 0;
}

void log_delivery(const char* agent, const char* name, const char* address) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    
    FILE *log_file = fopen("delivery.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    
    fprintf(log_file, "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] Express package delivered to %s in %s\n",
            local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
            local->tm_hour, local->tm_min, local->tm_sec,
            agent, name, address);
    fclose(log_file);
}

void* agent_thread(void* arg) {
    char agent_name = *(char*)arg;
    int shmid = shmget(1234, sizeof(SharedMemory), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        pthread_exit(NULL);
    }
    
    SharedMemory *shared_mem = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shared_mem == (void*)-1) {
        perror("shmat failed");
        pthread_exit(NULL);
    }
    
    while (1) {
        for (int i = 0; i < shared_mem->count; i++) {
            if (strcmp(shared_mem->orders[i].type, "Express") == 0 && 
                shared_mem->orders[i].delivered == 0) {
                
                // Simulate delivery
                sleep(1 + rand() % 3);
                
                shared_mem->orders[i].delivered = 1;
                sprintf(shared_mem->orders[i].delivered_by, "%c", agent_name);
                
                log_delivery(&agent_name, shared_mem->orders[i].name, shared_mem->orders[i].address);
            }
        }
        sleep(1);
    }
    
    shmdt(shared_mem);
    pthread_exit(NULL);
}

int main() {
    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Download the CSV file
    printf("Downloading delivery orders CSV file...\n");
    if (download_csv() != 0) {
        fprintf(stderr, "Failed to download CSV file\n");
        curl_global_cleanup();
        return 1;
    }
    printf("CSV file downloaded successfully\n");
    
    // Initialize shared memory
    int shmid = shmget(1234, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        curl_global_cleanup();
        return 1;
    }
    
    SharedMemory *shared_mem = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shared_mem == (void*)-1) {
        perror("shmat failed");
        curl_global_cleanup();
        return 1;
    }
    
    // Read CSV file
    FILE *csv_file = fopen(CSV_FILENAME, "r");
    if (csv_file == NULL) {
        perror("Failed to open CSV file");
        shmdt(shared_mem);
        curl_global_cleanup();
        return 1;
    }
    
    char line[500];
    shared_mem->count = 0;
    
    while (fgets(line, sizeof(line), csv_file) && shared_mem->count < MAX_ORDERS) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = 0;
        
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        strcpy(shared_mem->orders[shared_mem->count].name, token);
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strcpy(shared_mem->orders[shared_mem->count].address, token);
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strcpy(shared_mem->orders[shared_mem->count].type, token);
        
        shared_mem->orders[shared_mem->count].delivered = 0;
        shared_mem->orders[shared_mem->count].delivered_by[0] = '\0';
        
        shared_mem->count++;
    }
    
    fclose(csv_file);
    shmdt(shared_mem);
    
    // Create agent threads
    pthread_t agents[3];
    char agent_names[3] = {'A', 'B', 'C'};
    
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&agents[i], NULL, agent_thread, &agent_names[i]) != 0) {
            perror("Failed to create agent thread");
            curl_global_cleanup();
            exit(1);
        }
    }
    
    // Wait for threads to finish (they won't in this case)
    for (int i = 0; i < 3; i++) {
        pthread_join(agents[i], NULL);
    }
    
    curl_global_cleanup();
    return 0;
}