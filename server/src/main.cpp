#include "../include/server.h"
#include "../include/database.h"
#include "../include/logger.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>

// Global server instance for signal handling
Server* g_server = nullptr;

void signal_handler(int signum) {
    LOG_INFO("Received signal " + std::to_string(signum) + ", shutting down...");
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    std::cout << "=====================================" << std::endl;
    std::cout << "Online Testing System - Server" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Parse command line arguments
    int port = 8888; // Default port
    std::string db_path = "testing_app.db"; // Default database
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                port = std::atoi(argv[++i]);
            }
        } else if (arg == "--db" || arg == "-d") {
            if (i + 1 < argc) {
                db_path = argv[++i];
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --port, -p <port>    Server port (default: 8888)" << std::endl;
            std::cout << "  --db, -d <path>      Database path (default: testing_app.db)" << std::endl;
            std::cout << "  --help, -h           Show this help message" << std::endl;
            return 0;
        }
    }
    
    std::cout << "Port: " << port << std::endl;
    std::cout << "Database: " << db_path << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Initialize logger
    Logger::get_instance()->set_min_level(INFO);
    LOG_INFO("=== Server Starting ===");
    
    // Initialize database
    LOG_INFO("Initializing database...");
    Database db(db_path);
    if (!db.is_open()) {
        LOG_ERROR("Failed to open database");
        return 1;
    }
    
    if (!db.initialize()) {
        LOG_ERROR("Failed to initialize database");
        return 1;
    }
    
    LOG_INFO("Database initialized successfully");
    
    // Create server
    LOG_INFO("Creating server on port " + std::to_string(port) + "...");
    Server server(port, &db);
    g_server = &server;
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Start server (blocking)
    LOG_INFO("Starting server...");
    if (!server.start()) {
        LOG_ERROR("Failed to start server");
        return 1;
    }
    
    return 0;
}

