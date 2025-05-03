
extern Weapon available_weapons[];
extern int weapon_count;

extern Weapon* get_weapon_by_name(const char*);

#include <string.h>

typedef struct {
    char name[64];
    int damage;
    int price;
    char passive[256];
} Weapon;

Weapon available_weapons[] = {
    {"Rusty Sword", 10, 50, "None"},
    {"Flame Blade", 20, 100, "Burn 10% chance"},
    {"Ice Lance", 15, 80, "Freeze 15% chance"},
    {"Thunder Staff", 25, 120, "Chain Lightning 20% chance"},
    {"Shadow Dagger", 12, 60, "Dodge 5% chance"}
};

int weapon_count = 5;

Weapon* get_weapon_by_name(const char* name) {
    for (int i = 0; i < weapon_count; i++) {
        if (strcmp(available_weapons[i].name, name) == 0) {
            return &available_weapons[i];
        }
    }
    return NULL;
}