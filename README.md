# Sisop-2-2025-IT19

## Member

| No  | Nama                   | NRP        |
| --- | ---------------------- | ---------- |
| 1   | Ahmad Yazid Arifuddin  | 5027241040 |
| 2   | Muhammad Ziddan Habibi | 5027241122 |
| 3   | Andi Naufal Zaki       | 5027241059 |

## Soal 1

### Deskripsi Soal
Pada tahun 2045, dunia mengalami kekacauan infrastruktur siber. Sebagai mahasiswa ITS, kita harus kembali ke tahun 2025 untuk memanggil hacker "rootkids" dengan mengubah file teks rahasia menjadi file JPEG menggunakan sistem RPC server-client.

### Spesifikasi Program
1. **image_server.c**: Berjalan sebagai daemon, menerima koneksi dari client, memproses file teks, dan menyimpan hasil dekripsi.
2. **image_client.c**: Menghubungkan ke server, mengirim file teks untuk didekripsi, dan mendownload file JPEG hasil dekripsi.

### Solusi
#### **image_client.c**
- **Fungsi Utama**:
  - `download_and_extract_secrets()`: Mengunduh dan mengekstrak file teks rahasia dari Google Drive.
  - `connect_to_server()`: Membuat koneksi socket ke server.
  - `send_file_to_server()`: Mengirim file teks ke server untuk diproses.
  - `download_file_from_server()`: Mendownload file JPEG hasil dekripsi dari server.
- **Menu Interaktif**:
  - Pengguna dapat memilih untuk mengirim file atau mendownload file.
  - Setiap aksi dicatat dalam log server.

#### **image_server.c**
- **Fungsi Utama**:
  - `log_action()`: Mencatat setiap aksi ke dalam file log.
  - `reverse_string()`: Membalikkan string.
  - `hex_decode()`: Mengubah string hex menjadi data biner.
  - `decrypt_and_save()`: Memproses file teks (reverse + hex decode) dan menyimpan sebagai JPEG.
  - `send_file()`: Mengirim file JPEG ke client.
- **Daemon**:
  - Berjalan di background, menerima koneksi dari client, dan memproses permintaan.

### Cara Kerja
1. **Client**:
   - Mengunduh file teks rahasia.
   - Menghubungkan ke server.
   - Mengirim file teks untuk diproses.
   - Mendownload hasil dekripsi (JPEG).
2. **Server**:
   - Menerima koneksi dari client.
   - Memproses file teks (reverse + hex decode).
   - Menyimpan hasil sebagai JPEG.
   - Mengirim file JPEG kembali ke client saat diminta.

### Error Handling
- **Client**:
  - Gagal koneksi ke server.
  - File input tidak ditemukan.
- **Server**:
  - File tidak ditemukan saat dikirim ke client.
  - Server tetap berjalan meskipun terjadi error.

### Logging
- Setiap aksi dicatat dalam `server.log` dengan format:
![image](https://github.com/user-attachments/assets/1a1c5777-2432-47d3-9b0c-ab6f8fb0b5d0)
![image](https://github.com/user-attachments/assets/d8db673f-cb10-46b3-9813-eadd279e00c0)



## Soal 2
Sistem ini merupakan simulasi sistem pengiriman barang berbasis bahasa C untuk perusahaan ekspedisi **RushGo**, yang bertugas menangani pemrosesan pesanan **Express** dan **Regular** secara terpisah namun terintegrasi.

Terdiri dari dua komponen utama:

- `delivery_agent.c`: Menangani pengiriman **Express** secara otomatis oleh agen
- `dispatcher.c`: Mengelola pengiriman **Regular** secara manual oleh pengguna melalui terminal

---

## 🧩 Struktur Program

### 1. `delivery_agent.c`
Agen pengiriman otomatis yang bekerja berdasarkan file `delivery_order.csv` yang diunduh langsung dari Google Drive. Fitur-fitur:
- Mengunduh file CSV pesanan
- Menggunakan shared memory (`sysv shm`) untuk menyimpan data pesanan
- Tiga agen (`A`, `B`, `C`) dipicu dalam thread paralel untuk mengirim **Express**
- Mencatat semua pengiriman ke dalam `delivery.log`

#### 🔄 Proses Utama
1. Unduh file CSV dari URL Google Drive
2. Simpan data pesanan ke shared memory
3. Thread-thread agen membaca shared memory dan memproses pesanan Express
4. Menuliskan log pengiriman ke `delivery.log`

---

### 2. `dispatcher.c`
Program terminal untuk pengguna yang memungkinkan:
- Pengiriman manual untuk pesanan `Regular`
![image](https://github.com/user-attachments/assets/d69b99d5-71cd-4699-8352-4a175098c7fa)

- Melihat status pesanan berdasarkan nama pelanggan
![image](https://github.com/user-attachments/assets/64a4b255-a04f-43e8-8b3c-a12703455ad9)

- Menampilkan seluruh daftar pesanan beserta statusnya
![image](https://github.com/user-attachments/assets/3082dd80-0781-47de-8edf-4dddfab8a96c)


#### ⚙️ Perintah CLI
```bash
./dispatcher -deliver "Nama Pelanggan"   # Mengirim Regular order
./dispatcher -status "Nama Pelanggan"    # Cek status order
./dispatcher -list                       # Daftar seluruh pesanan
```
## Soal 3



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
```




### e)

![image](https://github.com/user-attachments/assets/a57007be-457a-4811-a929-d532bc7fe927)

Soal ini memerintahkan agar menambah fitur informasi untuk setiap dungeon, Fitur ini menampilkan daftar lengkap dungeon beserta nama, level minimum, reward (EXP, ATK, HP, DEF), dan key unik untuk masing-masing dungeon. 

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
Fitur ini ditambahkan agar admin bisa:
- Melihat daftar semua dungeon yang telah di-generate.
- Setiap dungeon memiliki:
- Nama dungeon (contoh: Demon Castle)
- Level minimum untuk masuk
- Reward: EXP, ATK, HP, DEF
- Key unik (sebagai ID dungeon)

Tampilan Menu Terminal
Gambar memperlihatkan:
- Sistem berbasis CLI (Command Line Interface).
- Menu dengan opsi seperti:
```
- Hunter Info
- Dungeon Info ← ini yang sedang dipilih
- Generate Dungeon
- Ban Hunter
- Reset Hunter
- Exit
```
Setelah memilih "Dungeon Info", sistem menampilkan detail dungeon pertama (Dungeon 1).

![Screenshot 2025-05-08 083024](https://github.com/user-attachments/assets/0d98b263-9fd1-435d-bd4e-40fbaf907e24)


### f)

![image](https://github.com/user-attachments/assets/b65b5393-48d9-4d37-943d-8b3c36143e25)


Fitur pada bagian ini merupakan sistem yang memungkinkan **hunter (pemain)** untuk melihat daftar dungeon yang **tersedia sesuai dengan level mereka**. Dungeon yang sebelumnya telah dibuat oleh sistem akan difilter berdasarkan level minimum yang diperlukan, sehingga hanya dungeon yang dapat diakses oleh hunter (sesuai atau di bawah level hunter) yang akan ditampilkan. Tujuannya adalah untuk membatasi akses dan menjaga keseimbangan permainan.

Tampilan pada gambar menunjukkan antarmuka CLI (Command Line Interface) dengan menu pilihan bernama "**koko's MENU**", di mana opsi pertama adalah "Dungeon List". Setelah memilih opsi tersebut, sistem menampilkan semua dungeon yang dapat diakses oleh hunter, lengkap dengan nama dan persyaratan level minimum. Dalam contoh yang ditampilkan, dua dungeon tersedia dan keduanya membutuhkan **Level 1+**, yang artinya hunter dengan level 1 ke atas dapat memasukinya.

Untuk mengimplementasikan fitur ini, dibutuhkan:

* **Data hunter**, termasuk levelnya.
* **Struktur data dungeon** seperti sebelumnya (mengandung nama dan level minimum).
* **Fungsi filter**, yang menyaring dan hanya menampilkan dungeon dengan `minimumLevel <= levelHunter`.
* Kontrol menu berbasis perulangan dan pengkondisian untuk menangani input dan output interaktif.

Fitur ini menekankan pentingnya **akses selektif berbasis level** agar pemain hanya berinteraksi dengan konten yang sesuai dengan kemampuannya di dalam game.


```c
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
```

![Screenshot 2025-05-08 225441](https://github.com/user-attachments/assets/20948505-8d34-44da-a058-513f54a609e7)


### g)

![image](https://github.com/user-attachments/assets/a5dd3f70-c8cd-4103-bb44-617cc94e523e)

Fitur yang ditampilkan pada bagian ini merupakan sistem **raid atau penaklukan dungeon** oleh hunter. Setelah melihat adanya hunter yang terlalu kuat, sistem menambahkan mekanisme baru di mana dungeon yang berhasil ditaklukkan akan **menghilang dari sistem**, menandakan bahwa dungeon tersebut telah dikuasai. Sebagai gantinya, hunter akan memperoleh **reward peningkatan status** seperti ATK, HP, DEF, dan EXP sesuai dengan atribut dari dungeon tersebut.

Tampilan antarmuka CLI menunjukkan daftar dungeon yang tersedia untuk di-raid (judul: **RAIDABLE DUNGEONS**). Hunter memilih salah satu dungeon berdasarkan daftar, dan setelah sukses melakukan raid, sistem menampilkan notifikasi bahwa raid berhasil dan menunjukkan reward yang diperoleh.

Fitur ini juga mencakup sistem level-up: jika **EXP hunter mencapai 500**, maka hunter akan **naik level** dan nilai EXP akan **di-reset ke 0**, meniru sistem RPG klasik yang berbasis progresi pengalaman.

Untuk mengimplementasikan fitur ini, diperlukan:

* **Struktur data hunter**, yang menyimpan level, exp, atk, hp, dan def.
* Mekanisme **penambahan stat dan exp** ketika raid berhasil.
* **Penghapusan dungeon** dari daftar setelah dikuasai.
* **Cek kondisi EXP**: jika mencapai 500 atau lebih, level bertambah dan exp di-reset.

Fitur ini memperkenalkan elemen **progresi dan pertumbuhan karakter**, serta menjaga dinamika permainan dengan menghapus dungeon yang telah ditaklukkan, mendorong hunter untuk terus mencari tantangan baru.


```c
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
```

![Screenshot 2025-05-08 225548](https://github.com/user-attachments/assets/2a321cf9-79e8-4325-bfd7-c13a6acfdede)



### h)

![image](https://github.com/user-attachments/assets/b8ad400d-c139-4082-8d63-49d26fe1000c)


Fitur yang dijelaskan pada bagian ini adalah **PvP (Player versus Player)**, yaitu sistem pertarungan antar hunter untuk meningkatkan kompetisi. Setiap hunter dapat memilih hunter lain sebagai lawan, dan kekuatan mereka dibandingkan berdasarkan **total stats**, yaitu penjumlahan dari **ATK + HP + DEF**.

Saat hunter memilih lawan, sistem akan menghitung kekuatan masing-masing. Jika **hunter menang**, ia akan mendapatkan **seluruh stats milik lawannya**, sedangkan lawan yang kalah akan **dihapus dari sistem**. Sebaliknya, jika hunter kalah, maka **hunter tersebut yang akan dihapus**, dan **stats-nya berpindah ke lawannya**.

Tampilan antarmuka menunjukkan informasi seperti:

* Nama target
* Power (kekuatan total) masing-masing hunter
* Hasil pertarungan (menang/kalah)
* Proses penghapusan dari shared memory dan transfer stats

Untuk membangun fitur ini, materi yang diperlukan mencakup:

* **Struktur data hunter** dengan atribut ATK, HP, DEF, dan identifier
* Fungsi untuk menghitung total stats dan membandingkan kekuatan
* **Pemilihan target** dari daftar hunter yang aktif
* **Mekanisme transfer stats dan penghapusan data**, yang bisa menggunakan operasi pada array/list atau memory sharing (jika dalam sistem multithread/proses)
* Penanganan input dan output berbasis CLI untuk tampilan interaktif

Fitur ini memberikan **elemen kompetitif dan risiko tinggi**, karena hunter bisa mendapatkan peningkatan besar atau kehilangan seluruh progres mereka, sehingga mendorong strategi dan perhitungan sebelum bertarung.


```c
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
```

![Screenshot 2025-05-08 225742](https://github.com/user-attachments/assets/95f49c33-6b88-4efe-b3cf-77954728cec9)


### i)

![image](https://github.com/user-attachments/assets/30a4ed9d-c265-463b-bd4a-08565d102082)

Deskripsi fitur ini adalah tentang **sistem kontrol dan penalti** terhadap hunter yang melakukan pelanggaran atau kecurangan dalam sistem. Untuk menjaga keseimbangan dan keadilan, Sung Jin Woo menambahkan **fitur ban/unban** hunter dari aktivitas **raid** atau **PvP battle**.

Fitur ini memungkinkan admin atau sistem untuk:

* **Mem-ban (melarang)** hunter tertentu agar tidak dapat melakukan raid atau bertarung dengan hunter lain.
* Menyimpan status ban secara **terstruktur**, misalnya dalam flag atau status boolean pada data hunter.
* **Meng-unban** hunter secara manual ketika masa percobaan atau penalti telah selesai, sehingga hunter dapat kembali mengakses fitur raid dan battle.

Implementasi teknis dari fitur ini dapat melibatkan:

* **Penambahan field `is_banned` atau `ban_status`** dalam struktur data hunter.
* Validasi di awal fungsi `raid()` dan `battle()` untuk memeriksa apakah hunter diban.
* Fungsi admin `ban_hunter()` dan `unban_hunter()` untuk mengatur status tersebut.
* Opsi pada menu admin agar bisa memilih hunter dan mengubah status ban-nya.

Fitur ini menambahkan elemen **pengawasan dan manajemen pengguna** dalam sistem, menjaga integritas permainan serta memberi fleksibilitas pada admin untuk menangani penyalahgunaan secara adil.


```c
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
```

![Screenshot 2025-05-08 230053](https://github.com/user-attachments/assets/d8b45660-404d-4653-bb6c-7f0ec4cba180)


### j)

![image](https://github.com/user-attachments/assets/dedbb1cc-78d6-4c1a-9de2-9db3eaee19ec)

Deskripsi fitur ini adalah tentang **reset stat hunter**, yaitu sistem yang memungkinkan seorang hunter mengatur ulang status mereka ke kondisi awal. Fitur ini dirancang sebagai bentuk **kesempatan kedua** bagi hunter yang mungkin sebelumnya terkena penalti, kalah dalam battle, atau ingin memulai kembali dari awal dengan kondisi standar.

Fungsi utama dari fitur ini:

* Mengembalikan **statistik hunter** (ATK, HP, DEF, EXP, dan Level) ke nilai **default awal**.
* Dapat diakses oleh admin atau sistem melalui menu khusus.
* Bisa menjadi bagian dari sistem pemulihan untuk hunter yang sebelumnya di-banned atau kehilangan semua stat dalam PvP.

Secara teknis, implementasinya melibatkan:

* Penyimpanan **nilai default stat** hunter dalam struktur data awal (misalnya: `base_stats`).
* Fungsi `reset_hunter_stats(hunter_id)` yang akan mengatur ulang semua nilai stat ke nilai default.
* Validasi agar hanya hunter tertentu (misalnya yang diizinkan oleh admin atau berada dalam kondisi tertentu) yang bisa di-reset.

Fitur ini memperkuat aspek **rehabilitasi dan fleksibilitas** sistem, memberi ruang bagi pemain untuk memperbaiki kesalahan tanpa harus membuat akun atau karakter baru.


```c
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
```

Sebelum di reset :
![image](https://github.com/user-attachments/assets/968ba67d-26df-42e0-be77-e25d4949a5e0)

setelah direset :

![image](https://github.com/user-attachments/assets/93c8207b-19a6-434a-9afb-4b8a16b25bfa)

pada hunter ese setiap poin kembali ke default awal setelah direset


### k)

![image](https://github.com/user-attachments/assets/692e7035-05cf-45c6-b129-ddc8fbaa9f46)

Deskripsi fitur ini adalah **notifikasi dungeon dinamis**, yaitu sistem yang secara otomatis memberikan informasi real-time kepada setiap hunter mengenai dungeon yang tersedia dan baru terbuka. Tujuan utama dari fitur ini adalah untuk membuat sistem terasa lebih **hidup, interaktif, dan tidak membosankan** bagi pengguna.

Fitur ini bekerja dengan cara:

* Menampilkan **notifikasi dungeon baru** yang muncul sesuai level minimum hunter.
* Notifikasi tersebut akan tampil secara berkala dan **berganti setiap 3 detik**, memberikan informasi yang bergiliran mengenai dungeon yang terbuka.
* Fitur ini aktif selama hunter berada di dalam sistem atau telah mengaktifkan menu notifikasi.

Secara teknis, implementasinya dapat berupa:

* Fungsi dengan loop `sleep(3)` yang menampilkan daftar dungeon dari array atau database.
* Sistem pengecekan level hunter agar hanya dungeon yang relevan yang ditampilkan.
* Tampilan teks yang muncul di atas menu utama hunter untuk memastikan selalu terlihat.

Fitur ini sangat berguna untuk **menjaga atensi pemain**, membantu mereka mengetahui peluang dungeon yang tersedia secara cepat, serta mendorong partisipasi dalam raid secara lebih aktif.


```c
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
```

![Screenshot 2025-05-08 230748](https://github.com/user-attachments/assets/ebf4ba4c-06bc-410b-8166-3d1b027c9adc)

notifikasi akan berjalan setiap 3 detik


### l)

![image](https://github.com/user-attachments/assets/7615670c-f27b-4659-bcda-48e6590789d2)















