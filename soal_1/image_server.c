#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER 4096
#define DATABASE_DIR "server/database"
#define LOG_FILE "server/server.log"

void create_directories() {
    mkdir("server", 0755);
    mkdir(DATABASE_DIR, 0755);
    mkdir("client", 0755);
    mkdir("client/secrets", 0755);
}

void log_message(const char *source, const char *action, const char *info) {
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

void handle_sigint(int sig) {
    log_message("Server", "SHUTDOWN", "Server terminated by signal");
    exit(0);
}

char* reverse_string(char *str) {
    if (!str) return NULL;
    
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    return str;
}

char* hex_decode(const char *hex_str, size_t *out_len) {
    if (!hex_str || strlen(hex_str) % 2 != 0) return NULL;
    
    size_t len = strlen(hex_str) / 2;
    char *bytes = malloc(len + 1);
    if (!bytes) return NULL;
    
    for (size_t i = 0; i < len; i++) {
        sscanf(hex_str + 2*i, "%2hhx", &bytes[i]);
    }
    bytes[len] = '\0';
    
    if (out_len) *out_len = len;
    return bytes;
}

char* process_text(const char *text) {
    printf("Debug: Original text length: %zu\n", strlen(text));
    
    // First, reverse the text
    char *reversed = strdup(text);
    if (!reversed) return NULL;
    
    reverse_string(reversed);
    printf("Debug: Reversed text: %s\n", reversed);
    
    // Check if the text contains only hex characters
    for (size_t i = 0; i < strlen(reversed); i++) {
        if (!isxdigit(reversed[i])) {
            printf("Error: Non-hex character found at position %zu: %c\n", i, reversed[i]);
            free(reversed);
            return NULL;
        }
    }
    
    // Hex decode
    size_t out_len;
    char *decoded = hex_decode(reversed, &out_len);
    free(reversed);
    
    if (!decoded) {
        printf("Error: Hex decoding failed\n");
        return NULL;
    }
    
    printf("Debug: Decoded data length: %zu bytes\n", out_len);
    return decoded;
}

int save_to_database(const char *data, size_t data_len, char *filename) {
    time_t now = time(NULL);
    snprintf(filename, 64, "%ld.jpeg", now);
    
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", DATABASE_DIR, filename);
    
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    
    fwrite(data, 1, data_len, file);
    fclose(file);
    
    return 1;
}

int send_file(int client_socket, const char *filename) {
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", DATABASE_DIR, filename);
    
    FILE *file = fopen(path, "rb");
    if (!file) {
        send(client_socket, "ERROR: File not found", 21, 0);
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Send file size first
    send(client_socket, &file_size, sizeof(file_size), 0);
    
    char buffer[MAX_BUFFER];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, MAX_BUFFER, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            fclose(file);
            return 0;
        }
    }
    
    fclose(file);
    return 1;
}

void handle_client(int client_socket) {
    char buffer[MAX_BUFFER];
    int bytes_received;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    getpeername(client_socket, (struct sockaddr*)&client_addr, &addr_len);
    char *client_ip = inet_ntoa(client_addr.sin_addr);
    
    while (1) {
        bytes_received = recv(client_socket, buffer, MAX_BUFFER - 1, 0);
        if (bytes_received <= 0) break;
        
        buffer[bytes_received] = '\0';
        
        if (strncmp(buffer, "DECRYPT:", 8) == 0) {
            char *text = buffer + 8;
            log_message("Client", "DECRYPT", "Text data");
            
            char *decoded = process_text(text);
            if (!decoded) {
                send(client_socket, "ERROR: Decryption failed", 24, 0);
                log_message("Server", "ERROR", "Decryption failed");
                continue;
            }
            
            char filename[64];
            if (save_to_database(decoded, strlen(decoded), filename)) {
                send(client_socket, filename, strlen(filename), 0);
                log_message("Server", "SAVE", filename);
            } else {
                send(client_socket, "ERROR: Failed to save file", 26, 0);
                log_message("Server", "ERROR", "Failed to save file");
            }
            
            free(decoded);
        }
        else if (strncmp(buffer, "DOWNLOAD:", 9) == 0) {
            char *filename = buffer + 9;
            log_message("Client", "DOWNLOAD", filename);
            
            if (send_file(client_socket, filename)) {
                log_message("Server", "UPLOAD", filename);
            } else {
                log_message("Server", "ERROR", "File upload failed");
            }
        }
        else if (strncmp(buffer, "EXIT", 4) == 0) {
            log_message("Client", "EXIT", "Client requested to exit");
            break;
        }
    }
    
    close(client_socket);
}

int main() {
    create_directories();
    signal(SIGINT, handle_sigint);
    
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
    
    #ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEPORT");
        exit(EXIT_FAILURE);
    }
    #endif
    
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    // Daemonize the server
    if (fork() != 0) {
        return 0;
    }
    
    log_message("Server", "STARTUP", "Server started");
    
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            log_message("Server", "ERROR", "Accept failed");
            continue;
        }
        
        if (fork() == 0) {
            close(server_fd);
            handle_client(client_socket);
            exit(0);
        }
        
        close(client_socket);
    }
    
    return 0;
}