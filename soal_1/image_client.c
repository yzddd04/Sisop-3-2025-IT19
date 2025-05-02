#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <curl/curl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SECRETS_DIR "secrets"
#define DOWNLOAD_URL "https://drive.google.com/uc?export=download&id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw"

// Fungsi untuk menulis data download ke file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Fungsi untuk mendownload file dari Google Drive
int download_secrets() {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char outfilename[FILENAME_MAX] = "secrets.zip";
    
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename, "wb");
        if (!fp) {
            fprintf(stderr, "Failed to create file %s\n", outfilename);
            return 0;
        }
        
        curl_easy_setopt(curl, CURLOPT_URL, DOWNLOAD_URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
            fclose(fp);
            remove(outfilename);
            curl_easy_cleanup(curl);
            return 0;
        }
        
        fclose(fp);
        curl_easy_cleanup(curl);
        
        // Ekstrak file zip
        char command[256];
        snprintf(command, sizeof(command), "unzip -o %s -d %s", outfilename, SECRETS_DIR);
        if (system(command) != 0) {
            fprintf(stderr, "Failed to extract secrets\n");
            return 0;
        }
        
        remove(outfilename);
        return 1;
    }
    return 0;
}

void create_secrets_dir() {
    struct stat st = {0};
    if (stat(SECRETS_DIR, &st) == -1) {
        mkdir(SECRETS_DIR, 0700);
    }
}

void display_menu() {
    printf("\nImage Decoder Client\n");
    printf("1. Send input file to server\n");
    printf("2. Download file from server\n");
    printf("3. Exit\n");
    printf(">> ");
}

int send_file_request(int sock, const char *filename) {
    write(sock, filename, 256);
    
    char response[64];
    read(sock, response, sizeof(response));
    
    if (strcmp(response, "ERROR") == 0) {
        printf("Error: Failed to process file\n");
        return 0;
    }
    
    printf("Server: Text decrypted and saved as %s\n", response);
    return 1;
}

int download_file(int sock, const char *filename) {
    write(sock, filename, 64);
    
    long file_size;
    if (read(sock, &file_size, sizeof(file_size)) <= 0) {
        printf("Error: Failed to get file size\n");
        return 0;
    }
    
    if (file_size <= 0) {
        printf("Error: File not found on server\n");
        return 0;
    }
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Failed to create file\n");
        return 0;
    }
    
    char buffer[BUFFER_SIZE];
    long remaining = file_size;
    
    while (remaining > 0) {
        int chunk = remaining > BUFFER_SIZE ? BUFFER_SIZE : remaining;
        int bytes_read = read(sock, buffer, chunk);
        
        if (bytes_read <= 0) {
            fclose(file);
            remove(filename);
            printf("Error: Failed to download file\n");
            return 0;
        }
        
        fwrite(buffer, 1, bytes_read, file);
        remaining -= bytes_read;
    }
    
    fclose(file);
    printf("Success! Image saved as %s\n", filename);
    return 1;
}

int main() {
    // Inisialisasi libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Buat folder secrets dan download file
    create_secrets_dir();
    printf("Downloading secret files...\n");
    if (!download_secrets()) {
        fprintf(stderr, "Warning: Failed to download secret files\n");
    } else {
        printf("Secret files downloaded successfully to %s/\n", SECRETS_DIR);
    }
    
    // Koneksi ke server
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        curl_global_cleanup();
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/Address not supported\n");
        curl_global_cleanup();
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        curl_global_cleanup();
        return -1;
    }
    
    printf("Connected to address %s:%d\n", SERVER_IP, PORT);
    
    int choice;
    char filename[256];
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        while (getchar() != '\n');
        
        write(sock, &choice, sizeof(choice));
        
        switch (choice) {
            case 1:
                printf("Enter the file name: ");
                scanf("%255s", filename);
                send_file_request(sock, filename);
                break;
                
            case 2:
                printf("Enter the file name: ");
                scanf("%255s", filename);
                download_file(sock, filename);
                break;
                
            case 3:
                printf("Exiting...\n");
                close(sock);
                curl_global_cleanup();
                return 0;
                
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
    
    close(sock);
    curl_global_cleanup();
    return 0;
}