#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>

#define localhost "127.0.0.1"
#define TCP_PORT "26256"

#define BUF_SIZE 2048
#define FLAG 0
#define LIS_MAX_SIZE 3000

#define CHECK_WALLET 0
#define TX_COINS 1
#define TX_LIST -1
#define STATS -3

using namespace std;

struct Transaction {
    int serial_number;
    string sender;
    string receiver;
    int amount;
};

struct QueryResult {
    int size;
    Transaction transaction_list[LIS_MAX_SIZE];
};

Transaction operation;
QueryResult operation_result;

int sockfd;
char buf[BUF_SIZE];

// starting the tcp connection
void start_tcp() {
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // port number is automatically dynamically assigned
    if ((rv = getaddrinfo(localhost, TCP_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }

    // create socket

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        // connecting

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
    }

    freeaddrinfo(servinfo); // all done with this structure
}

// refer to: https://beej.us/guide/bgnet/examples/client.c
// sending input over stringstream
void send() {
    stringstream ss;
    ss << operation.serial_number << " " << operation.sender << " " << operation.receiver << " " << operation.amount;
    if (send(sockfd, ss.str().c_str(),ss.str().length(), 0) == -1)
        perror("send");
}

// refer to: https://beej.us/guide/bgnet/examples/client.c
// sorting response into buf
void receive() {
    memset(&buf, 0, sizeof BUF_SIZE);
    if (recv(sockfd, buf, BUF_SIZE, 0) == -1) {
        perror("recv");
        exit(1);
    }
    //memset(&operation_result, 0, sizeof operation_result);
    //memcpy(&operation_result, buf, sizeof operation_result);
}

// function to check current balance
void check_wallet(string username) {
    operation.serial_number = CHECK_WALLET;
    operation.sender = username;
    send();
    cout << "\"" << username << "\" sent a balance enquiry request to the main server." << endl;
    receive();

    // record response using istringstream
    istringstream iss(buf);
    // in serverM, the result size is assigned as current balance
    iss >> operation_result.size;

    int balance = operation_result.size;
    cout << "The current balance of \"" << username << "\" is : " << balance << " alicoins." << endl;
}

// // function to transfer coins
void tx_coins(string sender, string receiver, int amount) {
    operation.serial_number = TX_COINS;
    operation.sender = sender;
    operation.receiver = receiver;
    operation.amount = amount;
    send();
    cout << "\"" << sender << "\" has requested to transfer " << amount << " coins to \"" << receiver << "\"." << endl;
    receive();

    istringstream iss(buf);
    iss >> operation_result.size;
    iss >> operation_result.transaction_list[0].amount;

    // result size here represents the status
    // 0 for insufficient balance
    // 1 for successfully transfering
    // -999 for both sender and receiver not in the network
    // -888 for sender not in the network
    // -777 for receiver not in the network
    bool check = false;
    if (operation_result.size == 0) {
        cout << "\"" << sender << "\" was unable to transfer " << amount << " alicoins to \"" << receiver << "\" because of insufficient balance." << endl;
        check = true;
    } else if (operation_result.size == 1) {
        cout << "\"" << sender << "\" successfully transferred " << amount << " alicoins to \"" << receiver << "\"." << endl;
        check = true;
    } else if (operation_result.size == -999) {
        cout << "Unable to proceed with the transaction as \"" << sender << "\" and " << "\"" << receiver << "\" are not part of the network." << endl;
    } else if (operation_result.size == -888) {
        cout << "Unable to proceed with the transaction as \"" << sender << "\" is not part of the network." << endl;
    } else if (operation_result.size == -777) {
        cout << "Unable to proceed with the transaction as \"" << receiver << "\" is not part of the network." << endl;
    }
    // getting current balance for server and output it
    int balance = operation_result.transaction_list[0].amount;
    if (check) {
        cout << "The current balance of \"" << sender << "\" is : " << balance << " alicoins." << endl;
    }
}

// this function is called for txlist, it will sort transactions by serial number
void get_and_sort_all_transactions() {
    operation.serial_number = TX_LIST;
    send();
    cout << "\"clientB\" sent a sorted list request to the main server." << endl;
    receive();

    istringstream iss(buf);
    iss >> operation_result.size;
}


void stats(string sender) {
    operation.serial_number = STATS;
    operation.sender = sender;
    send();
    cout << "\"" << sender << "\" sent a statistics enquiry request to the main server." << endl;
    receive();

    istringstream iss(buf);
    iss >> operation_result.size;

    cout << "\"" << sender << "\" statistics are the following.:" << endl;
    cout << "Rank" << "\t" << "Username" << "\t" << "NumofTransacions" << "\t" << "Total" << endl;
    for (int i = 0; i < operation_result.size; i++) {
        Transaction t = operation_result.transaction_list[i];
        iss >> t.serial_number;
        iss >> t.sender;
        iss >> t.amount;
        cout << i + 1 << "\t" << t.sender << "\t\t" << t.serial_number << "\t\t\t" << t.amount << endl;
    }
}


// refer to https://beej.us/guide/bgnet/examples/client.c
int main(int argc, char *argv[])
{
    start_tcp();
    cout << "The client B is up and running." << endl;

    // decide the operations by number of arguments
    while (true) {
        // txlist
        if (argc == 2 && strcmp(argv[1], "TXLIST") == 0) {
            get_and_sort_all_transactions();
            close(sockfd);
            return 0;
        }

        // check wallet
        else if (argc == 2) {
            check_wallet(argv[1]);
            close(sockfd);
            return 0;
        }

        // show stats
        else if (argc == 3 && strcmp(argv[2], "stats") == 0) {
            stats(argv[1]);
            close(sockfd);
            return 0;
        }

        // transfer coins
        else if (argc == 4) {
            tx_coins(argv[1], argv[2], atoi(argv[3]));
            close(sockfd);
            return 0;
        }
    }
    return 0;
}
