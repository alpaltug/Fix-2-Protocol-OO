#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctime>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define CLIENT_ID 1

std::string compId = "CLIENT1";

struct ExecutionReport {
    std::string clOrdId;
    std::string symbol;
    std::string side;
    int orderQty;
    std::string ordType;
    double price;
};

struct OrderCancelReject {
    std::string clOrdId;
    std::string origClOrdId;
    std::string ordStatus;
    std::string text;
};

struct NewOrderSingle {
    std::string instrument;
    std::string side;
    int quantity;
    double price;
};

struct OrderCancelRequest {
    std::string clOrdId;
};

class Client {
    int clientSeqNum;
    int clientSocket;
    std::FILE* logFile;
public:
    Client() : clientSeqNum(1) {}
    ~Client() {
        close(clientSocket);
        std::fclose(logFile);
    }
    // Place all the function definitions here
    // For example:
    void generateSendingTime(std::string& timeStr) {
        // Generate sending time logic here
    }

    // Remaining functions go here
};

int main() {
    Client client;
    // Use client object to call functions and handle operations
    // ...
    return 0;
}
