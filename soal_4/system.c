#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define MAX_HUNTERS 50
#define MAX_DUNGEONS 50

struct Hunter {
    char username[50];
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int banned;
    key_t shm_key;
};

struct Dungeon {
    char name[50];
    int min_level;
    int exp;
    int atk;
    int hp;
    int def;
    key_t shm_key;
};

struct SystemData {
    struct Hunter hunters[MAX_HUNTERS];
    int num_hunters;
    struct Dungeon dungeons[MAX_DUNGEONS];
    int num_dungeons;
    int current_notification_index;
};

key_t get_system_key() {
    return ftok("/tmp", 'S');
}

void generate_dungeon(struct SystemData *sys_data);

void ban_hunter(struct SystemData *sys_data, const char *username) {
    for (int i = 0; i < sys_data->num_hunters; i++) {
        if (strcmp(sys_data->hunters[i].username, username) == 0) {
            sys_data->hunters[i].banned = 1;
            printf("Hunter %s has been banned.\n", username);
            return;
        }
    }
    printf("Hunter not found.\n");
}

void unban_hunter(struct SystemData *sys_data, const char *username) {
    for (int i = 0; i < sys_data->num_hunters; i++) {
        if (strcmp(sys_data->hunters[i].username, username) == 0) {
            sys_data->hunters[i].banned = 0;
            printf("Hunter %s has been unbanned.\n", username);
            return;
        }
    }
    printf("Hunter not found.\n");
}

void reset_hunter_stats(struct SystemData *sys_data, const char *username) {
    for (int i = 0; i < sys_data->num_hunters; ++i) {
        if (strcmp(sys_data->hunters[i].username, username) == 0) {
            sys_data->hunters[i].level = 1;
            sys_data->hunters[i].exp = 0;
            sys_data->hunters[i].atk = 10;
            sys_data->hunters[i].hp = 100;
            sys_data->hunters[i].def = 5;
            printf("Stats for %s have been reset to initial values.\n", username);
            return;
        }
    }
    printf("Hunter not found.\n");
}



int main() {
    key_t key = get_system_key();
    int shmid = shmget(key, sizeof(struct SystemData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct SystemData *sys_data = (struct SystemData *) shmat(shmid, NULL, 0);
    if (sys_data == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Inisialisasi jika pertama kali
    if (sys_data->num_hunters == 0 && sys_data->num_dungeons == 0) {
        printf("[System] Inisialisasi data...\n");
        sys_data->num_hunters = 0;
        sys_data->num_dungeons = 0;
        sys_data->current_notification_index = 0;

        // Contoh dungeon default
        struct Dungeon dungeon = {"Dungeon Goblin", 1, 50, 5, 20, 2, ftok("/tmp", 'G')};
        sys_data->dungeons[0] = dungeon;
        sys_data->num_dungeons = 1;
    }

    printf("[System] Shared memory siap. Menunggu aktivitas hunter...\n");

    // Loop monitoring
    while (1) {
        int choice;
        printf("\n=== SYSTEM MENU ===\n");
        printf("1. Hunter Info\n");
        printf("2. Dungeon Info\n");
        printf("3. Generate Dungeon\n");
        printf("4. Ban Hunter\n");
        printf("5. Reset Hunter\n");
        printf("6. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar(); // Buang newline
    
        if (choice == 1) {
            printf("\n=== HUNTER INFO ===\n");
            for (int i = 0; i < sys_data->num_hunters; ++i) {
                struct Hunter *h = &sys_data->hunters[i];
                printf("Name: %s\tLevel: %d\tEXP: %d\tATK: %d\tHP: %d\tDEF: %d\tStatus: %s\n",
                       h->username, h->level, h->exp, h->atk, h->hp, h->def, h->banned ? "BANNED" : "ACTIVE");
            }
        } else if (choice == 2) {
                        printf("\n=== DUNGEON INFO ===\n");
                for (int i = 0; i < sys_data->num_dungeons; ++i) {
                    struct Dungeon *d = &sys_data->dungeons[i];
                    printf("\n[Dungeon %d]\n", i + 1);
                    printf("Name: %s\n", d->name);
                    printf("Minimum Level: %d\n", d->min_level);
                    printf("EXP Reward: %d\n", d->exp);
                    printf("ATK: %d\n", d->atk);
                    printf("HP: %d\n", d->hp);
                    printf("DEF: %d\n", d->def);
                    printf("Key: %d\n", d->shm_key);
                }
        } else if (choice == 3) {
            generate_dungeon(sys_data);
        } else if (choice == 4) {
            printf("\n1. Ban Hunter\n");
    printf("2. Unban Hunter\n");
    printf("Choice: ");
    int sub_choice;
    scanf("%d", &sub_choice);
    getchar(); // consume newline

    printf("Enter hunter username: ");
    char username[50];
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    if (sub_choice == 1) {
        ban_hunter(sys_data, username);
    } else if (sub_choice == 2) {
        unban_hunter(sys_data, username);
    } else {
        printf("Invalid choice.\n");
    }
        } else if (choice == 5) {
            printf("Enter hunter username to reset: ");
            char username[50];
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0;
            
            reset_hunter_stats(sys_data, username);

        } else if (choice == 6) {
            printf("Exiting...\n");
            break;
        }
    
        sleep(2);
    }
    return 0;
}

const char *dungeon_names[] = {
    "Double Dungeon",
    "Demon Castle",
    "Pyramid Dungeon",
    "Red Gate Dungeon",
    "Hunters Guild Dungeon",
    "Busan A-Rank Dungeon",
    "Insects Dungeon",
    "Goblins Dungeon",
    "D-Rank Dungeon",
    "Gwanak Mountain Dungeon",
    "Hapjeong Subway Station Dungeon"
};
#define DUNGEON_NAME_COUNT (sizeof(dungeon_names) / sizeof(dungeon_names[0]))

void generate_dungeon(struct SystemData *sys_data) {
    if (sys_data->num_dungeons >= MAX_DUNGEONS) {
        printf("Max dungeon reached!\n");
        return;
    }

    struct Dungeon new_dungeon;

    // Pilih nama dungeon secara acak dari daftar
    int random_index = rand() % DUNGEON_NAME_COUNT;
    snprintf(new_dungeon.name, sizeof(new_dungeon.name), "%s", dungeon_names[random_index]);

    // Random nilai dengan rentang sesuai ketentuan
    new_dungeon.min_level = (rand() % 5) + 1;          // 1 - 5
    new_dungeon.atk       = (rand() % 51) + 100;       // 100 - 150
    new_dungeon.hp        = (rand() % 51) + 50;        // 50 - 100
    new_dungeon.def       = (rand() % 26) + 25;        // 25 - 50
    new_dungeon.exp       = (rand() % 151) + 150;      // 150 - 300

    new_dungeon.shm_key = ftok("/tmp", 'G' + sys_data->num_dungeons);

    sys_data->dungeons[sys_data->num_dungeons++] = new_dungeon;

    printf("\nDungeon generated!\n");
    printf("Name: %s\n", new_dungeon.name);
    printf("Minimum Level: %d\n", new_dungeon.min_level);
}
