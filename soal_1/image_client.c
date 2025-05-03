#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include <curl/curl.h>
#include <errno.h>
#include <limits.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define SECRETS_DIR "client/secrets/"
#define GDRIVE_URL "https://drive.google.com/uc?export=download&id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw"
#define ZIP_FILE "secrets.zip"
#define MAX_PATH 4096

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int download_and_extract_secrets() {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    
    printf("Preparing secret files...\n");
    
    // Create target directory if it doesn't exist
    if (mkdir(SECRETS_DIR, 0755) == -1 && errno != EEXIST) {
        perror("Error creating secrets directory");
        return 0;
    }
    
    // Check if files already exist
    DIR *dir = opendir(SECRETS_DIR);
    if (dir) {
        struct dirent *entry;
        int file_count = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, "input_")) {
                file_count++;
            }
        }
        closedir(dir);
        
        if (file_count >= 5) {  // Assuming there are 5 input files
            printf("Secret files already exist, skipping download.\n");
            return 1;
        }
    }

    printf("Downloading secret files...\n");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (!curl) {
        fprintf(stderr, "Error initializing curl\n");
        return 0;
    }

    fp = fopen(ZIP_FILE, "wb");
    if (!fp) {
        perror("Error creating zip file");
        curl_easy_cleanup(curl);
        return 0;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, GDRIVE_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
        fclose(fp);
        remove(ZIP_FILE);
        curl_easy_cleanup(curl);
        return 0;
    }
    
    fclose(fp);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    printf("Extracting files...\n");
    char command[512];
    snprintf(command, sizeof(command), "unzip -j -o %s -d %s", ZIP_FILE, SECRETS_DIR);
    
    int ret = system(command);
    if (ret != 0) {
        fprintf(stderr, "Error extracting files (code %d)\n", ret);
        remove(ZIP_FILE);
        return 0;
    }
    
    if (remove(ZIP_FILE) != 0) {
        perror("Warning: Could not remove zip file");
    }
    
    printf("Secret files ready!\n");
    return 1;
}

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    return sock;
}

void send_file_to_server(int sock, const char *filename) {
    char fullpath[MAX_PATH];
    snprintf(fullpath, sizeof(fullpath), "%s%s", SECRETS_DIR, filename);
    
    FILE *fp = fopen(fullpath, "r");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (!file_content) {
        perror("Memory allocation failed");
        fclose(fp);
        return;
    }

    if (fread(file_content, 1, file_size, fp) != (size_t)file_size) {
        perror("Error reading file");
        free(file_content);
        fclose(fp);
        return;
    }
    file_content[file_size] = '\0';
    fclose(fp);

    int option = 1;
    if (write(sock, &option, sizeof(option)) != sizeof(option)) {
        perror("Error sending option");
        free(file_content);
        return;
    }

    size_t filename_len = strlen(filename);
    if (write(sock, &filename_len, sizeof(filename_len)) != sizeof(filename_len) ||
        write(sock, filename, filename_len) != filename_len) {
        perror("Error sending filename");
        free(file_content);
        return;
    }

    if (write(sock, &file_size, sizeof(file_size)) != sizeof(file_size) ||
        write(sock, file_content, file_size) != (ssize_t)file_size) {
        perror("Error sending file content");
        free(file_content);
        return;
    }

    free(file_content);

    char response[8] = {0};
    if (read(sock, response, sizeof(response) - 1) <= 0) {
        perror("Error reading response");
        return;
    }

    if (strcmp(response, "SUCCESS") == 0) {
        char output_filename[64] = {0};
        if (read(sock, output_filename, sizeof(output_filename) - 1) <= 0) {
            perror("Error reading output filename");
            return;
        }
        printf("Server: Text decrypted and saved as %s\n", output_filename);
    } else {
        printf("Error: Failed to process file\n");
    }
}

void download_file_from_server(int sock, const char *filename) {
    int option = 2;
    if (write(sock, &option, sizeof(option)) != sizeof(option)) {
        perror("Error sending option");
        return;
    }

    size_t filename_len = strlen(filename);
    if (write(sock, &filename_len, sizeof(filename_len)) != sizeof(filename_len) ||
        write(sock, filename, filename_len) != filename_len) {
        perror("Error sending filename");
        return;
    }

    long file_size;
    if (read(sock, &file_size, sizeof(file_size)) != sizeof(file_size)) {
        perror("Error reading file size");
        return;
    }

    if (file_size <= 0) {
        printf("Error: File not found on server\n");
        return;
    }

    char *file_content = malloc(file_size);
    if (!file_content) {
        perror("Memory allocation failed");
        return;
    }

    size_t total_read = 0;
    while (total_read < (size_t)file_size) {
        ssize_t bytes_read = read(sock, file_content + total_read, file_size - total_read);
        if (bytes_read <= 0) {
            perror("Error receiving file content");
            free(file_content);
            return;
        }
        total_read += bytes_read;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error creating output file");
        free(file_content);
        return;
    }

    if (fwrite(file_content, 1, file_size, fp) != (size_t)file_size) {
        perror("Error writing to file");
        fclose(fp);
        free(file_content);
        return;
    }

    fclose(fp);
    free(file_content);
    printf("Success! Image saved as %s\n", filename);
}

void print_menu() {
    printf("\n--------------------------------\n");
    printf("|      Image Decoder Client     |\n");
    printf("--------------------------------\n\n");
    printf("1. Send input file to server\n");
    printf("2. Download file from server\n");
    printf("3. Exit\n");
    printf(">> ");
}

int main() {
    printf("./client\n");
    
    // Automatically download and prepare secret files
    if (!download_and_extract_secrets()) {
        printf("Failed to prepare secret files. Exiting.\n");
        return 1;
    }
    
    int sock = connect_to_server();
    if (sock < 0) {
        return 1;
    }
    
    printf("Connected to address 127.0.0.1:%d\n", PORT);
    
    int running = 1;
    while (running) {
        print_menu();
        
        int option;
        if (scanf("%d", &option) != 1) {
            printf("Invalid input\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        
        switch (option) {
            case 1: {
                printf("Enter the file name: ");
                char filename[256];
                if (scanf("%255s", filename) != 1) {
                    printf("Invalid filename\n");
                    break;
                }
                send_file_to_server(sock, filename);
                break;
            }
            case 2: {
                printf("Enter the file name: ");
                char filename[256];
                if (scanf("%255s", filename) != 1) {
                    printf("Invalid filename\n");
                    break;
                }
                download_file_from_server(sock, filename);
                break;
            }
            case 3: {
                int option = 3;
                write(sock, &option, sizeof(option));
                running = 0;
                break;
            }
            default:
                printf("Invalid option\n");
        }
    }
    
    close(sock);
    return 0;
}