#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <memory>
#include <cstdio> // for fopen, fclose
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h> // for htons, htonl, ...

#define MAX_PENDING_REQUESTS 100
#define MAX_MESSAGES 100
#define MAX_ORDERS 100
#define BUFFER_SIZE 1024

struct NewOrderSingle {
    std::string clOrdId;
    int clientId;
    std::string instrument;
    std::string side;
    int quantity;
    double price;
};

struct MarketData {
    std::string instrument;
    double lastPx;
};

class ClientInfo {
    int clientId;
    int lastSeqNum;
    std::string compId;
    std::unique_ptr<FILE, decltype(&fclose)> logFile;

public:
    ClientInfo(int clientId, std::string compId)
    : clientId(clientId), compId(compId), logFile(std::unique_ptr<FILE, decltype(&fclose)>(nullptr, &fclose)) {
        char filename[100];
        sprintf(filename, "%s_%d.txt", compId.c_str(), clientId);
        logFile.reset(fopen(filename, "w"));
    }

    int getClientId() const { return clientId; }
    int getLastSeqNum() const { return lastSeqNum; }
    void setLastSeqNum(int seqNum) { lastSeqNum = seqNum; }
    void setClientId(int id) { clientId = id; }

    std::string getCompId() const { return compId; }
    void setCompId(std::string id) { compId = id; }

    void appendLog(const std::string& entry) {
        if (logFile) {
            fprintf(logFile.get(), "%s\n", entry.c_str());
        }
    }
};

class Server {
    int serverSocket;
    int serverSeqNum;
    std::vector<ClientInfo> clientList;
    std::vector<NewOrderSingle> buyOrders;
    std::vector<NewOrderSingle> sellOrders;
    std::vector<MarketData> marketDataList;
    std::vector<std::string> sentMessages;
    int clientCount;
    int marketDataCount;

public:
    Server(int port, int serverSeqNum)
    : serverSeqNum(serverSeqNum) {
        clientList.reserve(MAX_PENDING_REQUESTS);
        buyOrders.reserve(MAX_ORDERS);
        sellOrders.reserve(MAX_ORDERS);
        marketDataList.reserve(MAX_ORDERS);
        sentMessages.reserve(MAX_MESSAGES);

        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Cannot open socket\n";
            exit(1);
        }

        // Bind socket
        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Cannot bind socket\n";
            exit(1);
        }

        // Listen on socket
        if (listen(serverSocket, MAX_PENDING_REQUESTS) < 0) {
            std::cerr << "Cannot listen on socket\n";
            exit(1);
        }
    }

    // Add additional member functions as necessary
    // For example:
    // void acceptConnections() {...}

    void processNewOrderSingle(const NewOrderSingle& order) {
        // Store the order in the corresponding vector
        if (order.side == "buy") {
            buyOrders.push_back(order);
        } else if (order.side == "sell") {
            sellOrders.push_back(order);
        }

        // Here you might also want to:
        // - Match the new order against the existing ones
        // - Update the market data
        // - Send messages to the clients
        // - etc.
    }

    void processMarketData(const MarketData& data) {
        // Update the market data
        // Here we simply add the new data to the list,
        // but in a real application, you would probably want to
        // keep the market data for each instrument in a map or another data structure
        marketDataList.push_back(data);

        // Here you might also want to:
        // - Notify the clients about the market data update
        // - Reevaluate the existing orders
        // - etc.
    }logic to process the market data
    }

    void acceptConnections() {
        while (true) {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(serverSocket, (struct sockaddr *) &client_addr, &client_len);

            if (client_socket < 0) {
                std::cerr << "Cannot accept client connection\n";
                continue;
            }

            // Create a new ClientInfo and add it to clientList
            int clientId = clientCount++;
            clientList.push_back(ClientInfo(clientId, "COMP_ID"));
        }

    // void handleClient(ClientInfo& client) {...}

    void handleClient(ClientInfo& client) {
        char buffer[BUFFER_SIZE];
        int n = read(client.getClientId(), buffer, BUFFER_SIZE);
        if (n < 0) {
            std::cerr << "Error reading from client socket\n";
        } else {
            // Parse the message and process it
            // Depending on the structure of your messages, 
            // you might need a more sophisticated parsing logic
            std::string msg(buffer, n);
            if (msg.find("NewOrderSingle") != std::string::npos) {
                NewOrderSingle order;
                // Parse the order details from the message
                processNewOrderSingle(order);
            } else if (msg.find("MarketData") != std::string::npos) {
                MarketData data;
                // Parse the market data from the message
                processMarketData(data);
            }
            client.setLastSeqNum(client.getLastSeqNum() + 1);
            client.appendLog(msg);
            }
        }
    //...
    };
};


int main() {
    int port = 8080; // Change as per your needs
    int serverSeqNum = 1;

    Server server(port, serverSeqNum);
    // Other operations

     std::thread acceptThread([&server](){ server.acceptConnections(); });

    // Continually handle client requests in the main thread
    while (true) {
        for (ClientInfo& client : server.clientList) {
            server.handleClient(client);
        }
    }

    return 0;
}
