***TCP Server-Client Programs***

**Included functions**
- List files on server
- Transfer file from server to client

**Instructions**
*Compile*
- Run `gcc server.c -o server` and `gcc client.c -o client`
- Move `server` and `client` executable files to desired folders

*Run and Use*
- On different terminal windows, navigate to the folders where `server` and `client` executable files are stored, and run with `./server` and `./client` respectively
- On client machine, there are two available commands
    - `listall` lists all files in the folder where `server` is being run
    - `send <filename>` copies a file from the server to the client