# Sisop-3-2025-IT19


| Nama                     | Nrp        |
| ------------------------ | ---------- |
| Ahmad Yazid Arifuddin    | 5027241040 |
|                          |            |

## SOAL 1 (Ahmad Yazid Arifuddin)
 
# Building and Running the System
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
