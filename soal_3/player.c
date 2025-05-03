#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "dungeon.c"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080


// struct dari dungeon.c
typedef struct {
    char name[64];
    int damage;
    int price;
    char passive[256];
} Weapon;

typedef struct {
    int gold;
    Weapon current_weapon;
    int base_damage;
    int enemies_defeated;
    Weapon inventory[5];
} PlayerStats;

typedef struct {
    int id;
} SessionID;

typedef struct {
    char name[64];
    int hp;
    int reward_gold;
} Enemy;

typedef struct {
    int damage;
    int remaining_hp;
    int defeated;
    int reward_gold;
    char passive_effect[256];
} AttackResult;

void show_menu() {
    printf("\n=== The Lost Dungeon ===\n");
    printf("1. Show Player Stats\n");
    printf("2. Visit Shop\n");
    printf("3. View Inventory & Equip Weapon\n");
    printf("4. Battle Mode\n");
    printf("5. Exit\n");
    printf("Choose: ");
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    int choice;
    do {
        show_menu();
        scanf("%d", &choice);
        char buffer[1024];

        switch (choice) {
            case 1: {
                send(sock, "GET_STATS", 10, 0);
                PlayerStats stats;
                recv(sock, &stats, sizeof(stats), 0);
                printf("Gold: %d\n", stats.gold);
                printf("Current Weapon: %s (Damage: %d)\n", stats.current_weapon.name, stats.current_weapon.damage);
                printf("Base Damage: %d\n", stats.base_damage);
                printf("Enemies Defeated: %d\n", stats.enemies_defeated);
                break;
            }
            case 2: {
                Weapon *weapons = (Weapon*)malloc(sizeof(Weapon) * 5);
                // Simulasi mengambil senjata dari server
                // (Dalam implementasi nyata, ini dikirim lewat socket)
                for (int i = 0; i < 5; i++) {
                    strcpy(weapons[i].name, available_weapons[i].name);
                    weapons[i].damage = available_weapons[i].damage;
                    weapons[i].price = available_weapons[i].price;
                    strcpy(weapons[i].passive, available_weapons[i].passive);
                }
                for (int i = 0; i < 5; i++) {
                    printf("%d. %s (Price: %d, Damage: %d, Passive: %s)\n",
                           i+1, weapons[i].name, weapons[i].price, weapons[i].damage, weapons[i].passive);
                }
                char weapon_name[256];
                printf("Enter weapon name to buy: ");
                scanf("%s", weapon_name);
                sprintf(buffer, "BUY_WEAPON %s", weapon_name);
                send(sock, buffer, strlen(buffer), 0);
                int success;
                recv(sock, &success, sizeof(success), 0);
                if (success) printf("Purchase successful!\n");
                else printf("Failed to buy weapon.\n");
                break;
            }
            case 3: {
                send(sock, "GET_STATS", 10, 0);
                PlayerStats stats;
                recv(sock, &stats, sizeof(stats), 0);
                printf("\n=== Inventory & Equipment ===\n");
                printf("Equipped Weapon: %s (Damage: %d, Passive: %s)\n", 
                       stats.current_weapon.name, stats.current_weapon.damage, stats.current_weapon.passive);
                printf("Inventory:\n");
                for (int i = 0; i < 5; i++) {
                    if (strlen(stats.inventory[i].name) > 0) {
                        printf(" [%d] %s (Damage: %d, Passive: %s)\n", i+1, stats.inventory[i].name, stats.inventory[i].damage, stats.inventory[i].passive);
                    }
                }
                printf("\nEnter weapon number to equip (or 0 to cancel): ");
                int choice;
                scanf("%d", &choice);
                if (choice > 0 && choice <= 5) {
                    sprintf(buffer, "EQUIP_WEAPON %d", choice);
                    send(sock, buffer, strlen(buffer), 0);
                    int success;
                    recv(sock, &success, sizeof(success), 0);
                    if (success) printf("Weapon equipped successfully!\n");
                    else printf("Failed to equip weapon.\n");
                }
                break;
            }
            case 4: {
                send(sock, "START_BATTLE", 12, 0);
                Enemy e;
                recv(sock, &e, sizeof(e), 0);
                printf("Fighting %s (HP: %d)\n", e.name, e.hp);
                char action;
                do {
                    printf("Enter 'a' to attack or 'e' to exit: ");
                    scanf(" %c", &action);
                    if (action == 'a') {
                        send(sock, "ATTACK", 6, 0);
                        AttackResult res;
                        recv(sock, &res, sizeof(res), 0);
                        printf("Dealt %d damage! Remaining HP: %d\n", res.damage, res.remaining_hp);
                        if (res.defeated) {
                            printf("Defeated! Reward: %d gold\n", res.reward_gold);
                        }
                        if (strlen(res.passive_effect)) {
                            printf("Passive Effect: %s\n", res.passive_effect);
                        }
                    }
                } while (action != 'e');
                break;
            }
            case 5:
                printf("Exiting...\n");
                close(sock);
                exit(0);
        }
    } while (choice != 5);

    return 0;
}
