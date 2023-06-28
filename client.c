#include <iostream>
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

    std::string generateClientOrderId() {
        return "ORD" + std::to_string(clientSeqNum++);
    }

public:
    Client() : clientSeqNum(1), clientSocket(-1), logFile(nullptr) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(clientSocket == -1) {
            std::cerr << "Socket creation failed.\n";
            exit(1);
        }

        logFile = fopen("logfile.txt", "w");
        if(logFile == nullptr) {
            std::cerr << "Could not open log file.\n";
            exit(1);
        }
    }

    ~Client() {
        close(clientSocket);
        if (logFile) {
            std::fclose(logFile);
        }
    }
    
    void connectToServer() {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        
        if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
            std::cerr << "inet_pton() failed\n";
            exit(1);
        }

        if (connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "connect() failed\n";
            exit(1);
        }
    }

    void sendNewOrderSingle(const NewOrderSingle& order) {
        std::string msg = "8=FIX.4.2;9=154;35=D;49=" + compId + ";56=SERVER;34=" + 
                          std::to_string(clientSeqNum) + ";52=00000000;11=" + 
                          generateClientOrderId() + ";21=1;40=2;55=" + 
                          order.instrument + ";54=" + order.side + ";38=" + 
                          std::to_string(order.quantity) + ";44=" + 
                          std::to_string(order.price) + ";10=000;";
        sendMessage(msg);
    }

    void sendOrderCancelRequest(const OrderCancelRequest& request) {
        std::string msg = "8=FIX.4.2;9=154;35=F;49=" + compId + ";56=SERVER;34=" + 
                          std::to_string(clientSeqNum) + ";52=00000000;41=" + 
                          request.clOrdId + ";11=" + 
                          generateClientOrderId() + ";54=1;55=MSFT;10=000;";
        sendMessage(msg);
    }

     void processReceivedMessage(const std::string& msg) {
        // Simple parsing of FIX messages
        std::map<std::string, std::string> tags;
        std::stringstream ss(msg);
        std::string item;
        while (std::getline(ss, item, ';')) {
            auto delimiterPos = item.find("=");
            auto tag = item.substr(0, delimiterPos);
            auto value = item.substr(delimiterPos + 1, item.size() - delimiterPos - 1);
            tags[tag] = value;
        }

        // Interpret message based on MsgType(35)
        if (tags["35"] == "8") {  // Execution Report
            ExecutionReport execReport = {
                tags["11"],    // clOrdId
                tags["55"],    // symbol
                tags["54"],    // side
                std::stoi(tags["38"]), // orderQty
                tags["40"],    // ordType
                std::stod(tags["44"])  // price
            };
            std::cout << "Received execution report for order " << execReport.clOrdId << '\n';
            // Do something with execReport...

        } else if (tags["35"] == "9") {  // Order Cancel Reject
            OrderCancelReject cancelReject = {
                tags["11"],  // clOrdId
                tags["41"],  // origClOrdId
                tags["39"],  // ordStatus
                tags["58"]   // text
            };
            std::cout << "Received order cancel reject for order " << cancelReject.clOrdId << '\n';
            // Do something with cancelReject...
        }
    }



    void connectToServer() {
        std::cout << "Connecting to server...\n";

        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        
        if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
            std::cerr << "inet_pton() failed\n";
            exit(1);
        }

        if (connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "connect() failed\n";
            exit(1);
        }

        std::cout << "Connected to server.\n";
    }

    void run() {
        connectToServer();

        std::cout << "Starting trading session...\n";

        // Send a new order
        NewOrderSingle order = {"MSFT", "2", 100, 50.0};
        sendNewOrderSingle(order);

        // Wait for execution report
        std::string msg = receiveMessage();
        processReceivedMessage(msg);

        // Send order cancellation
        OrderCancelRequest cancelRequest = {order.clOrdId};
        sendOrderCancelRequest(cancelRequest);

        // Wait for cancel reject or execution report
        msg = receiveMessage();
        processReceivedMessage(msg);

        std::cout << "Trading session ended.\n";

        //test

        while(true) {
            std::string msg = receiveMessage();
            processReceivedMessage(msg);
        }
    }
};
    
    void sendMessage(const std::string& msg) {
        if(send(clientSocket, msg.c_str(), msg.size(), 0) < 0) {
            std::cerr << "Send message failed.\n";
            exit(1);
        }
    }
    
    std::string receiveMessage() {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        int len = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if(len < 0) {
            std::cerr << "Receive failed.\n";
            exit(1);
        }
        return std::string(buffer, len);
    }

    void generateSendingTime(std::string& timeStr) {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer [80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, sizeof(buffer), "%Y%m%d-%H:%M:%S", timeinfo);
        timeStr = std::string(buffer);
    }

    // Remaining functions go here
};

int main() {
    Client client;
    client.connectToServer();
    // Use client object to call functions and handle operations
    // ...
    return 0;
}
