# Sisop-2-2025-IT19

## Member

| No  | Nama                   | NRP        |
| --- | ---------------------- | ---------- |
| 1   | Ahmad Yazid Arifuddin  | 5027241040 |
| 2   | Muhammad Ziddan Habibi | 5027241122 |
| 3   | Andi Naufal Zaki       | 5027241059 |


## Soal_4

### a)

![image](https://github.com/user-attachments/assets/041b65ff-9a0a-4628-bdce-11ba3dd9b64c)

Soal ini memerintahkan agar membuat 2 file C yaitu, file system.c dan file hunter.c, yang mana file system.c merupakan shared memory utama, sehingga file hunter.c tidak dapat dijalankan kecuali setelah file system.c sudah jalan.

![image](https://github.com/user-attachments/assets/b4f65acf-3be9-40be-a2cd-191849cc7e7d)

### b)

![image](https://github.com/user-attachments/assets/799bcb11-faf6-445b-8c83-7c1147ecbfe2)

Pada bagian ini diperintahkan agar membuat fitur registrasi dan log in yang nantinya dta hunter itu akan disimpan di shared memory tersendiri yang terhubung dengan system. data dan statistik awal hunter mencakup (Level=1, EXP=0, ATK=10, HP=100, DEF=5).

![image](https://github.com/user-attachments/assets/c9abdfba-6032-4eab-a0a8-12cbd13e04e7)

### c)

![image](https://github.com/user-attachments/assets/e9a0b978-81b5-42d7-ae5a-670c3f2db61d)

Pada bagian ini diperintahkan untuk membuat system yang menampilkan info dari semua hunter. infonya berupa nama hunter, level, exp, atk, hp, def, dan status (banned atau tidak).

![image](https://github.com/user-attachments/assets/c0001311-d9ae-4e38-b74c-e898aea7ab74)

ini merupakan hasil yang ditampilkanya


### d)

![image](https://github.com/user-attachments/assets/16fe0bf7-8ab5-4db5-b3bf-2bfc4b866b88)

Dibagian ini diperintahkan membuat program agar dapat membuat dungeon dengan nama yang acak, level minimal hunter, dan stat rewards dengan nilai:
🏆Level Minimal : 1 - 5
⚔️ATK : 100 - 150 Poin
❤️HP  : 50 - 100 Poin
🛡️DEF : 25 - 50 Poin
🌟EXP : 150 - 300 Poin

![image](https://github.com/user-attachments/assets/366ead93-ae81-444f-869c-594d8680bcfd)

dengan kode berikut :
```void generate_dungeon(struct SystemData *sys_data);``` mendeklarasikan fungsi generate_dungeon

```c
else if (choice == 2) {
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
                    }
```




### e)

![image](https://github.com/user-attachments/assets/a57007be-457a-4811-a929-d532bc7fe927)



### f)

![image](https://github.com/user-attachments/assets/b65b5393-48d9-4d37-943d-8b3c36143e25)

### g)

![image](https://github.com/user-attachments/assets/a5dd3f70-c8cd-4103-bb44-617cc94e523e)


### h)

![image](https://github.com/user-attachments/assets/b8ad400d-c139-4082-8d63-49d26fe1000c)


### i)

![image](https://github.com/user-attachments/assets/30a4ed9d-c265-463b-bd4a-08565d102082)

### j)

![image](https://github.com/user-attachments/assets/dedbb1cc-78d6-4c1a-9de2-9db3eaee19ec)

### k)

![image](https://github.com/user-attachments/assets/692e7035-05cf-45c6-b129-ddc8fbaa9f46)

### l)

![image](https://github.com/user-attachments/assets/7615670c-f27b-4659-bcda-48e6590789d2)













