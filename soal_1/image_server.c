#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define DATABASE_DIR "database"
#define LOG_FILE "server.log"

void log_action(const char *source, const char *action, const char *info) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
        fclose(log_file);
    }
}

void create_database_dir() {
    struct stat st = {0};
    if (stat(DATABASE_DIR, &st) == -1) {
        mkdir(DATABASE_DIR, 0700);
    }
}

char* reverse_string(const char *str) {
    int length = strlen(str);
    char *reversed = malloc(length + 1);
    
    for (int i = 0; i < length; i++) {
        reversed[i] = str[length - 1 - i];
    }
    reversed[length] = '\0';
    
    return reversed;
}

char* hex_decode(const char *hex_str) {
    size_t len = strlen(hex_str);
    if (len % 2 != 0) return NULL;
    
    size_t final_len = len / 2;
    char *decoded = malloc(final_len + 1);
    
    for (size_t i = 0, j = 0; j < final_len; i += 2, j++) {
        sscanf(hex_str + i, "%2hhx", &decoded[j]);
    }
    decoded[final_len] = '\0';
    
    return decoded;
}

int save_decrypted_file(const char *text_data, char *filename) {
    char *reversed = reverse_string(text_data);
    if (!reversed) return 0;
    
    char *decoded = hex_decode(reversed);
    free(reversed);
    if (!decoded) return 0;
    
    time_t now = time(NULL);
    snprintf(filename, 64, "%ld.jpeg", now);
    
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", DATABASE_DIR, filename);
    
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        free(decoded);
        return 0;
    }
    
    fwrite(decoded, 1, strlen(decoded), file);
    fclose(file);
    free(decoded);
    
    return 1;
}

int send_file_to_client(int client_socket, const char *filename) {
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", DATABASE_DIR, filename);
    
    FILE *file = fopen(filepath, "rb");
    if (!file) return 0;
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    write(client_socket, &file_size, sizeof(file_size));
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (write(client_socket, buffer, bytes_read) != bytes_read) {
            fclose(file);
            return 0;
        }
    }
    
    fclose(file);
    return 1;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int choice;
    
    while (1) {
        if (read(client_socket, &choice, sizeof(choice)) <= 0) break;
        
        if (choice == 1) {
            char filename[256];
            read(client_socket, filename, sizeof(filename));
            
            FILE *file = fopen(filename, "r");
            if (!file) {
                write(client_socket, "ERROR", 6);
                log_action("Server", "ERROR", "Failed to open input file");
                continue;
            }
            
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char *file_content = malloc(file_size + 1);
            fread(file_content, 1, file_size, file);
            file_content[file_size] = '\0';
            fclose(file);
            
            char saved_filename[64];
            if (save_decrypted_file(file_content, saved_filename)) {
                write(client_socket, saved_filename, sizeof(saved_filename));
                log_action("Client", "DECRYPT", "Text data");
                log_action("Server", "SAVE", saved_filename);
            } else {
                write(client_socket, "ERROR", 6);
                log_action("Server", "ERROR", "Failed to decrypt file");
            }
            
            free(file_content);
        }
        else if (choice == 2) {
            char filename[64];
            read(client_socket, filename, sizeof(filename));
            
            if (send_file_to_client(client_socket, filename)) {
                log_action("Client", "DOWNLOAD", filename);
                log_action("Server", "UPLOAD", filename);
            } else {
                write(client_socket, "ERROR", 6);
                log_action("Server", "ERROR", "Failed to find file for download");
            }
        }
        else if (choice == 3) {
            log_action("Client", "EXIT", "Client requested to exit");
            break;
        }
    }
    
    close(client_socket);
}

int main() {
    create_database_dir();
    
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server running on port %d...\n", PORT);
    log_action("Server", "START", "Server started");
    
    if (fork() != 0) {
        return 0;
    }
    
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        log_action("Server", "CONNECT", "New client connected");
        
        if (fork() == 0) {
            close(server_socket);
            handle_client(client_socket);
            exit(0);
        }
        
        close(client_socket);
    }
    
    close(server_socket);
    return 0;
}