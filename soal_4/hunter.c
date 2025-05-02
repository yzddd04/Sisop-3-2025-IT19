#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>

#define MAX_HUNTERS 50

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
    struct Dungeon dungeons[50];
    int num_dungeons;
    int current_notification_index;
};

key_t get_system_key() {
    return ftok("/tmp", 'S');
}

void show_available_dungeons(struct SystemData *sys_data, struct Hunter *hunter) {
    printf("\n=== AVAILABLE DUNGEONS ===\n");
    int found = 0;
    for (int i = 0; i < sys_data->num_dungeons; ++i) {
        struct Dungeon *d = &sys_data->dungeons[i];
        if (hunter->level >= d->min_level) {
            printf("%d. %s\t(Level %d+)\n", i + 1, d->name, d->min_level);
            found = 1;
        }
    }
    if (!found) {
        printf("No available dungeons for your level (%d).\n", hunter->level);
    }
    printf("\nPress enter to continue...");
    getchar();
}

void show_available_dungeons(struct SystemData *sys_data, struct Hunter *hunter);

void raid_dungeon(struct SystemData *sys_data, struct Hunter *hunter) {
    printf("\n=== RAIDABLE DUNGEONS ===\n");
    int available_indexes[50];
    int count = 0;

    for (int i = 0; i < sys_data->num_dungeons; i++) {
        if (hunter->level >= sys_data->dungeons[i].min_level) {
            printf("%d. %s\t(Level %d+)\n", count + 1, sys_data->dungeons[i].name, sys_data->dungeons[i].min_level);
            available_indexes[count++] = i;
        }
    }

    if (count == 0) {
        printf("No dungeon available to raid.\n");
        printf("\nPress enter to continue...");
        getchar();
        return;
    }

    printf("Choose Dungeon: ");
    int choice;
    scanf("%d", &choice);
    getchar();

    if (choice < 1 || choice > count) {
        printf("Invalid choice.\n");
        return;
    }

    int dungeon_index = available_indexes[choice - 1];
    struct Dungeon raided = sys_data->dungeons[dungeon_index];

    printf("\nRaid success! Gained:\n");
    printf("ATK: %d\n", raided.atk);
    printf("HP: %d\n", raided.hp);
    printf("DEF: %d\n", raided.def);
    printf("EXP: %d\n", raided.exp);

    hunter->atk += raided.atk;
    hunter->hp += raided.hp;
    hunter->def += raided.def;
    hunter->exp += raided.exp;

    if (hunter->exp >= 500) {
        hunter->level++;
        hunter->exp = 0;
        printf("LEVEL UP! You are now Level %d\n", hunter->level);
    }

    // Hapus dungeon dari list
    for (int i = dungeon_index; i < sys_data->num_dungeons - 1; i++) {
        sys_data->dungeons[i] = sys_data->dungeons[i + 1];
    }
    sys_data->num_dungeons--;

    printf("\nPress enter to continue...");
    getchar();
}

void hunter_battle(struct SystemData *sys_data, int attacker_index) {
    struct Hunter *attacker = &sys_data->hunters[attacker_index];

    printf("\n=== PVP LIST ===\n");
    int count = 0;

    for (int i = 0; i < sys_data->num_hunters; i++) {
        if (i == attacker_index || sys_data->hunters[i].banned) continue;

        struct Hunter *opponent = &sys_data->hunters[i];
        int total_power = opponent->atk + opponent->hp + opponent->def;
        printf("%d. %s - Total Power: %d\n", i + 1, opponent->username, total_power);
        count++;
    }

    if (count == 0) {
        printf("No available opponents.\n");
        printf("Press enter to continue...");
        getchar();
        return;
    }

    printf("Target: ");
    int target_input;
    scanf("%d", &target_input);
    getchar();  // Consume newline
    int defender_index = target_input - 1;

    if (defender_index < 0 || defender_index >= sys_data->num_hunters || defender_index == attacker_index) {
        printf("Invalid target.\n");
        printf("Press enter to continue...");
        getchar();
        return;
    }

    struct Hunter *defender = &sys_data->hunters[defender_index];

    int attacker_power = attacker->atk + attacker->hp + attacker->def;
    int defender_power = defender->atk + defender->hp + defender->def;

    printf("\nYou chose to battle %s\n", defender->username);
    printf("Your Power: %d\n", attacker_power);
    printf("Opponent's Power: %d\n", defender_power);

    if (attacker_power >= defender_power) {
        printf("Deleting defender's shared memory (shmid: %d)\n", shmget(defender->shm_key, sizeof(struct Hunter), 0666));
        shmctl(shmget(defender->shm_key, sizeof(struct Hunter), 0666), IPC_RMID, NULL);

        attacker->atk += defender->atk;
        attacker->hp += defender->hp;
        attacker->def += defender->def;

        // Remove defender from list
        for (int i = defender_index; i < sys_data->num_hunters - 1; i++) {
            sys_data->hunters[i] = sys_data->hunters[i + 1];
        }
        sys_data->num_hunters--;

        printf("Battle won! You acquired %s's stats\n", defender->username);
    } else {
        printf("Deleting your shared memory (shmid: %d)\n", shmget(attacker->shm_key, sizeof(struct Hunter), 0666));
        shmctl(shmget(attacker->shm_key, sizeof(struct Hunter), 0666), IPC_RMID, NULL);

        defender->atk += attacker->atk;
        defender->hp += attacker->hp;
        defender->def += attacker->def;

        // Remove attacker from list
        for (int i = attacker_index; i < sys_data->num_hunters - 1; i++) {
            sys_data->hunters[i] = sys_data->hunters[i + 1];
        }
        sys_data->num_hunters--;

        printf("You lost the battle. You were removed from the system.\n");
        // Exit program if attacker is deleted
        exit(0);
    }

    printf("\nPress enter to continue...");
    getchar();
}

void show_dungeon_notifications(struct SystemData *sys_data) {
    pid_t pid = fork();

    if(pid < 0) {
        perror("fork failed");
        return;
    }

    if(pid == 0){
        while(1){
            system("cleaar");
            printf("=== HUNTER SYSTEM ===\n");

            struct Dungeon *d = &sys_data->dungeons[sys_data->current_notification_index];
            printf("%s Dungeon for minimum level %d TERBUKA WOOOOOOOOOYYYY\n",
                d->name, d->min_level);
                    sys_data->current_notification_index = 
                    (sys_data->current_notification_index + 1) % sys_data->num_dungeons;

                sleep(3);
            }
            exit(0);
        } else {
            printf("Press enter to stop notifications...\n");
            getchar();
            kill(pid, SIGKILL);  // Hentikan child process setelah enter
        }  

}

int main() {
    key_t key = get_system_key();
    int shmid = shmget(key, sizeof(struct SystemData), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct SystemData *sys_data = (struct SystemData *) shmat(shmid, NULL, 0);
    if (sys_data == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }
    
    int choice;
    char username[50];
    int logged_in_index = -1;

    while (1) {
        printf("=== HUNTER MENU ===\n1. Register\n2. Login\n3. Exit\nChoice: ");
        scanf("%d", &choice);
        getchar();  // consume newline

        if (choice == 1) { // Register
            printf("Username: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0;

            // Cek duplikat
            int exists = 0;
            for (int i = 0; i < sys_data->num_hunters; i++) {
                if (strcmp(sys_data->hunters[i].username, username) == 0) {
                    exists = 1;
                    break;
                }
            }

            if (exists) {
                printf("Username already taken!\n");
                continue;
            }

            struct Hunter new_hunter;
            strcpy(new_hunter.username, username);
            new_hunter.level = 1;
            new_hunter.exp = 0;
            new_hunter.atk = 10;
            new_hunter.hp = 100;
            new_hunter.def = 5;
            new_hunter.banned = 0;
            new_hunter.shm_key = ftok("/tmp", username[0]);  // Key unik

            sys_data->hunters[sys_data->num_hunters++] = new_hunter;

            printf("Registration success!\n");

        } else if (choice == 2) { // Login
            printf("Username: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0;

            int found = 0;
            for (int i = 0; i < sys_data->num_hunters; i++) {
                if (strcmp(sys_data->hunters[i].username, username) == 0) {
                    logged_in_index = i;
                    found = 1;
                    break;
                }
            }

            if (found) {
                printf("Welcome, %s!\n", username);// Lanjut ke menu selanjutnya
            }
         struct Hunter *current_hunter = &sys_data->hunters[logged_in_index];

            while (1) {
                printf("\n=== %s's MENU ===\n", current_hunter->username);
                printf("1. Dungeon List\n");
                printf("2. Dungeon Raid\n");
                printf("3. Hunters Battle\n");
                printf("4. Notification\n");
                printf("5. Exit\n");
                printf("Choice: ");
                scanf("%d", &choice);
                getchar();  // Buang newline

                if (choice == 1) {
                    show_available_dungeons(sys_data, current_hunter);
                } else if (choice == 2) {
                    if (current_hunter->banned) {
                        printf("You are currently banned from raiding dungeons.\n");
                        continue;
                    }
                    raid_dungeon(sys_data, current_hunter);
                } else if (choice == 3) {
                    if (current_hunter->banned) {
                        printf("You are banned from doing raids or PvP.\n");
                        continue; // Kembali ke menu utama hunter
                    }                    
                    hunter_battle(sys_data, logged_in_index);
                    break;
                } else if (choice == 4) {
                    show_dungeon_notifications(sys_data);
                } else if (choice == 5) {
                    printf("Logging out...\n");
                    break;
                } else {
                    printf("Invalid choice!\n");
                }
            }
        } else if (choice == 3) {
            break;
        }
    }

    // Lanjutkan ke HUNTER SYSTEM di sini jika login berhasil

    return 0;
}