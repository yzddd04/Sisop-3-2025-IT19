#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 8080


typedef struct {
    char name[64];
    int damage;
    int price;
    char passive[256];
} Weapon;

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

#define MAX_SESSIONS 10
#define PORT 8080

typedef struct {
    SessionID id;
    PlayerStats stats;
    Enemy current_enemy;
    int has_enemy;
} Session;

Session sessions[MAX_SESSIONS];
int session_count = 0;

// Senjata dari shop.c
extern Weapon available_weapons[];
extern int weapon_count;
extern Weapon* get_weapon_by_name(const char*);

// Fungsi Bantuan
Session* find_session(int id) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].id.id == id) {
            return &sessions[i];
        }
    }
    return NULL;
}

// Logika Game
SessionID register_session() {
    static int next_id = 0;
    SessionID sid = {next_id++};
    Session *s = &sessions[sid.id];
    s->id = sid;
    s->stats.gold = 100;
    s->stats.base_damage = 10;
    s->stats.enemies_defeated = 0;
    s->has_enemy = 0;
    return sid;
}

PlayerStats get_stats(SessionID sid) {
    Session *s = find_session(sid.id);
    return s->stats;
}

Weapon* list_weapons() {
    return available_weapons;
}

int buy_weapon(SessionID sid, const char* name) {
    Session *s = find_session(sid.id);
    if (!s) return 0;

    // Contoh penggunaan di dungeon.c
Weapon* w = get_weapon_by_name(name);
if (!w) return 0;

// Menampilkan daftar senjata
for (int i = 0; i < weapon_count; i++) {
    printf("%d. %s (Price: %d, Damage: %d, Passive: %s)\n",
           i + 1, available_weapons[i].name,
           available_weapons[i].price,
           available_weapons[i].damage,
           available_weapons[i].passive);
}
    return 0;
}

Enemy start_battle(SessionID sid) {
    Session *s = find_session(sid.id);
    Enemy e = {"Goblin", 50 + rand() % 151, 10 + rand() % 41};
    s->current_enemy = e;
    s->has_enemy = 1;
    return e;
}

AttackResult attack(SessionID sid) {
    AttackResult res = {0};
    Session *s = find_session(sid.id);
    if (!s || !s->has_enemy) return res;

    Enemy *e = &s->current_enemy;
    PlayerStats *p = &s->stats;

    int damage = p->base_damage + p->current_weapon.damage;
    damage = damage * (0.9 + 0.2 * ((double)rand() / RAND_MAX));
    if (rand() % 100 < 15) damage *= 2;

    // Efek pasif
    if (strcmp(p->current_weapon.passive, "Burn 10% chance") == 0) {
        if (rand() % 100 < 10) {
            damage += 10;
            strcpy(res.passive_effect, "Burn applied!");
        }
    }

    e->hp -= damage;
    res.damage = damage;
    res.remaining_hp = e->hp;
    res.defeated = e->hp <= 0;
    if (res.defeated) {
        p->gold += e->reward_gold;
        p->enemies_defeated += 1;
        res.reward_gold = e->reward_gold;
        s->has_enemy = 0;
    }
    return res;
}

int equip_weapon(SessionID sid, int index) {
    Session *s = find_session(sid.id);
    if (!s || index < 0 || index >= 5) return 0;

    Weapon w = s->stats.inventory[index];
    if (strlen(w.name) == 0) return 0;

    s->stats.current_weapon = w;
    return 1;
}

// Server Socket
void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    char buffer[1024];
    SessionID sid = register_session();

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0) break;

        char command[64], param[256];
        sscanf(buffer, "%s", command);

        if (strcmp(command, "GET_STATS") == 0) {
            PlayerStats stats = get_stats(sid);
            send(client_socket, &stats, sizeof(stats), 0);
        } else if (strcmp(command, "BUY_WEAPON") == 0) {
            sscanf(buffer, "%*s %s", param);
            int success = buy_weapon(sid, param);
            send(client_socket, &success, sizeof(success), 0);
        } else if (strcmp(command, "START_BATTLE") == 0) {
            Enemy e = start_battle(sid);
            send(client_socket, &e, sizeof(e), 0);
        } else if (strcmp(command, "ATTACK") == 0) {
            AttackResult res = attack(sid);
            send(client_socket, &res, sizeof(res), 0);
        } else if (strcmp(command, "EQUIP_WEAPON") == 0) {
            int index;
            sscanf(buffer, "%*s %d", &index);
            int success = equip_weapon(sid, index - 1);
            send(client_socket, &success, sizeof(success), 0);
        }
    }

    close(client_socket);
    return NULL;
}

int main() {
    srand(time(NULL));
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server running on port %d\n", PORT);
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, &new_socket);
        pthread_detach(thread_id);
    }

    return 0;
}