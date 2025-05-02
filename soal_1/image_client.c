#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <curl/curl.h>
#include <zip.h>
#include <libgen.h> // For basename function

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define MAX_BUFFER 4096
#define SECRETS_URL "https://drive.google.com/uc?export=download&id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw"
#define SECRETS_ZIP "client/secrets.zip"

void create_directories() {
    mkdir("client", 0755);
    mkdir("client/secrets", 0755);
}

// Callback function for curl download
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int download_secrets() {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return 0;
    }

    fp = fopen(SECRETS_ZIP, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to create file %s\n", SECRETS_ZIP);
        curl_easy_cleanup(curl);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, SECRETS_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    printf("Downloading secrets.zip...\n");
    res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
        return 0;
    }

    return 1;
}

int extract_zip() {
    int err = 0;
    struct zip *za = zip_open(SECRETS_ZIP, 0, &err);
    if (!za) {
        fprintf(stderr, "Failed to open zip file: %d\n", err);
        return 0;
    }

    int num_entries = zip_get_num_entries(za, 0);
    printf("Extracting %d files...\n", num_entries);

    for (int i = 0; i < num_entries; i++) {
        const char *name = zip_get_name(za, i, 0);
        if (!name) continue;

        // Get just the filename without directory path
        char *base_name = basename((char *)name);
        if (strcmp(base_name, ".") == 0 || strcmp(base_name, "..") == 0) {
            continue; // Skip parent and current directory entries
        }

        char output_path[256];
        snprintf(output_path, sizeof(output_path), "client/secrets/%s", base_name);

        struct zip_file *zf = zip_fopen_index(za, i, 0);
        if (!zf) {
            fprintf(stderr, "Failed to open file %s in zip\n", name);
            continue;
        }

        FILE *out = fopen(output_path, "wb");
        if (!out) {
            fprintf(stderr, "Failed to create output file %s\n", output_path);
            zip_fclose(zf);
            continue;
        }

        char buf[100];
        int bytes_read;
        while ((bytes_read = zip_fread(zf, buf, sizeof(buf))) > 0) {
            fwrite(buf, 1, bytes_read, out);
        }

        fclose(out);
        zip_fclose(zf);
        printf("Extracted: %s\n", output_path);
    }

    zip_close(za);
    return 1;
}

void check_and_download_secrets() {
    // Check if secrets directory is empty
    DIR *dir = opendir("client/");
    if (dir) {
        struct dirent *ent;
        int count = 0;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.') count++;
        }
        closedir(dir);

        if (count >= 5) { // Assuming there are 5 input files
            printf("Secrets already downloaded\n");
            return;
        }
    }

    if (!download_secrets()) {
        fprintf(stderr, "Failed to download secrets\n");
        return;
    }

    if (!extract_zip()) {
        fprintf(stderr, "Failed to extract secrets\n");
        return;
    }

    // Remove the zip file after extraction
    remove(SECRETS_ZIP);
    printf("Secrets downloaded and extracted successfully\n");
}

void display_menu() {
    printf("\nImage Decoder Client | The Legend of Rootkids\n");
    printf("--------------------------------------------\n");
    printf("1. Send input file to server\n");
    printf("2. Download file from server\n");
    printf("3. Exit\n");
    printf(">> ");
}

int read_file(const char *filename, char *buffer, size_t buffer_size) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    size_t bytes_read = fread(buffer, 1, buffer_size - 1, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return 1;
}

int save_file(const char *filename, const char *data, size_t data_size) {
    char path[128];
    snprintf(path, sizeof(path), "client/%s", filename);
    
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    
    fwrite(data, 1, data_size, file);
    fclose(file);
    return 1;
}

int connect_to_server() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/Address not supported\n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }
    
    return sock;
}

void send_file_to_server(int sock, const char *filename) {
    char buffer[MAX_BUFFER];
    if (!read_file(filename, buffer, MAX_BUFFER)) {
        printf("Error: Could not read file %s\n", filename);
        return;
    }
    
    printf("Debug: File content (first 100 chars):\n%.100s\n", buffer);
    
    char message[MAX_BUFFER + 8];
    snprintf(message, sizeof(message), "DECRYPT:%s", buffer);
    
    send(sock, message, strlen(message), 0);
    
    char response[MAX_BUFFER];
    int bytes_received = recv(sock, response, MAX_BUFFER - 1, 0);
    if (bytes_received <= 0) {
        printf("Server connection error\n");
        return;
    }
    
    response[bytes_received] = '\0';
    
    if (strncmp(response, "ERROR:", 6) == 0) {
        printf("Server: %s\n", response + 6);
    } else {
        printf("Server: Text decrypted and saved as %s\n", response);
    }
}

void download_file_from_server(int sock, const char *filename) {
    char message[MAX_BUFFER];
    snprintf(message, sizeof(message), "DOWNLOAD:%s", filename);
    
    send(sock, message, strlen(message), 0);
    
    // First receive the file size
    long file_size;
    if (recv(sock, &file_size, sizeof(file_size), 0) <= 0) {
        printf("Error receiving file size\n");
        return;
    }
    
    if (file_size <= 0) {
        char error_msg[MAX_BUFFER];
        int bytes_received = recv(sock, error_msg, MAX_BUFFER - 1, 0);
        if (bytes_received > 0) {
            error_msg[bytes_received] = '\0';
            printf("Server: %s\n", error_msg);
        }
        return;
    }
    
    char *file_data = malloc(file_size + 1);
    if (!file_data) {
        printf("Memory allocation failed\n");
        return;
    }
    
    size_t total_received = 0;
    while (total_received < file_size) {
        int bytes_received = recv(sock, file_data + total_received, file_size - total_received, 0);
        if (bytes_received <= 0) {
            printf("Error receiving file data\n");
            free(file_data);
            return;
        }
        total_received += bytes_received;
    }
    
    if (save_file(filename, file_data, file_size)) {
        printf("Success! Image saved as %s\n", filename);
    } else {
        printf("Error saving file\n");
    }
    
    free(file_data);
}

int main() {
    create_directories();
    
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Download and extract secrets automatically
    check_and_download_secrets();
    
    int choice;
    char filename[256];
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        getchar(); // Consume newline
        
        int sock = connect_to_server();
        if (sock < 0) {
            printf("Failed to connect to server\n");
            continue;
        }
        
        switch (choice) {
            case 1:
                printf("Enter the file name: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0';
                
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "client/secrets/%s", filename);
                send_file_to_server(sock, full_path);
                break;
                
            case 2:
                printf("Enter the file name: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0';
                download_file_from_server(sock, filename);
                break;
                
            case 3:
                send(sock, "EXIT", 4, 0);
                close(sock);
                printf("Exiting...\n");
                curl_global_cleanup();
                return 0;
                
            default:
                printf("Invalid choice\n");
        }
        
        close(sock);
    }
    
    curl_global_cleanup();
    return 0;
}