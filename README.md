# UNIX-Socket-programming

Note that you can directly use make all to compile all the files. However, if you would like to compile each file individually, you will need to add -pthread when compiling the serverM.cpp file $g++ -o serverM serverM.cpp -pthread, as I used pthread to create multiple threads cocurrently. You can just run the code by ./fileoutput after make all.

I did complete Phase 4 for extra credit.

Port# serverA: 21256, serverB: 22256, serverC: 23256, serverM: 24256, clientA: 25256, clientB: 26256.

In this assignment, I created 2 clients, 1 main server, and 3 backend servers. I used TCP between clients and main server and UDP between the main server and backend servers.

In clientA and B, they will take command line inputs for checking wallet, transferring coins, sorting transactions to alichain.txt, and showing stats of one user sorting by number of transactions made (desc). I use number of arguments to determine which operation to perform, and pass the transaction details to serverM through TCP.

In serverM, I take the information from client. Then I map operations into integers and use serial number (the integer) to determine the operation to process. In all 4 operations I need to pass the information again to the other servers through UDP, then calculating balance, creating sorted lists, making stats table, etc. I also check if the sender and receiver exist in the network in this file.

In serverA, B, and C, they also use the integer I defined to determine operations. They read the block files and look for related transactions of one username and pass all the information to serverM to make the sorted transaction list. And they also record the newly transferring transactions to block files. Many information then passes to serverM and pass back to client to output on screen message.

I have used Beej’s socket programming tutorial at several places in my code, I have marked them in my code.

I used two ways to store message, one is a structure of serial number, sender, receiver, and amount, the second one is a structure of size and the array of previous structure. The first one is to store single transaction and input, the second one is to store lots of transactions and deal with operations in this project. I didn't pass these structures for messaging though. I used istringstream and stringstream to pass and extract the useful information from these structures.

The project will probably fail if the input integer is too high, as the type I put is int. And if block has more than 3000 lines, it will also fail, but instructor in Piazza said there will be no more than 1000 lines in a file, so it's fine.

I used part of Beej's code on TCP send and receive, as well as UDP and TCP listener.
