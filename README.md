# Sisop-3-2025-IT19


| Nama                     | Nrp        |
| ------------------------ | ---------- |
| Ahmad Yazid Arifuddin    | 5027241040 |
| Muhammad Ziddan Habibi   | 5027241122 |
| Andi Naufal Zaki         | 5027241059 |

## SOAL 1 (Ahmad Yazid Arifuddin)
 
1. First, create the necessary directory structure:
`mkdir -p server/database client/secrets`

2. Compile the server:
`gcc image_server.c -o server/image_server`

3. Compile the client:
`gcc image_client.c -o client/image_client -lcurl`

4. Start the server (as a daemon):
`./server/image_server`

5. Run the client:
`./client/image_client`

kill server
`sudo lsof -i :8080`
kemudian 
`kill <PID>`

## SOAL 2 (Ahmad Yazid Arifuddin)
1. Compile file delivery_agent
`gcc delivery_agent.c -o delivery_agent -lcurl`

2. Comple file dispatcher
`gcc dispatcher.c -o dispatcher -lcurl`

3. Run code delivery_agent nya
`./delivery_agent` kemudian akan mendownload csv nya secara otomatis dan melakukan express

4. Run code dispatcher
`./dispatcher -list` untuk melihat list kiriman
`./dispatcher -deliver <nama>` untuk mengirim yang masih pending
`./dispatcher -status <status>` untuk mengecek status kiriman



