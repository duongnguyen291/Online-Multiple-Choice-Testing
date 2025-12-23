#include "../include/websocket_gateway.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>

WebSocketGateway* g_gateway = nullptr;

void signal_handler(int signum) {
    std::cout << "Received signal " << signum << ", shutting down..." << std::endl;
    if (g_gateway) {
        g_gateway->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    std::cout << "=====================================" << std::endl;
    std::cout << "WebSocket Gateway for Online Testing" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    int port = 8080;                  // WebSocket port
    std::string backend_host = "127.0.0.1";  // Backend server host
    int backend_port = 8888;           // Backend server port
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                port = std::atoi(argv[++i]);
            }
        } else if (arg == "--backend-host" || arg == "-bh") {
            if (i + 1 < argc) {
                backend_host = argv[++i];
            }
        } else if (arg == "--backend-port" || arg == "-bp") {
            if (i + 1 < argc) {
                backend_port = std::atoi(argv[++i]);
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --port, -p <port>              WebSocket port (default: 8080)" << std::endl;
            std::cout << "  --backend-host, -bh <host>    Backend server host (default: 127.0.0.1)" << std::endl;
            std::cout << "  --backend-port, -bp <port>    Backend server port (default: 8888)" << std::endl;
            std::cout << "  --help, -h                     Show this help message" << std::endl;
            return 0;
        }
    }
    
    std::cout << "WebSocket Port: " << port << std::endl;
    std::cout << "Backend: " << backend_host << ":" << backend_port << std::endl;
    std::cout << "=====================================" << std::endl;
    
    WebSocketGateway gateway(port, backend_host, backend_port);
    g_gateway = &gateway;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (!gateway.start()) {
        std::cerr << "Failed to start gateway" << std::endl;
        return 1;
    }
    
    gateway.run();
    
    return 0;
}
