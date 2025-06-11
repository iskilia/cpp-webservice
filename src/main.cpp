#include <httplib.h>
#include <iostream>
#include <string>
#include <chrono>
#include <format>
#include <memory>
#include <thread>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std::chrono;

class WebService {
private:
    std::unique_ptr<httplib::Server> server;
    const std::string version = "1.0.0";
    const int port;
    
    // Service start time for uptime calculation
    std::chrono::steady_clock::time_point start_time;

public:
    explicit WebService(int port = 8080) : port(port) {
        server = std::make_unique<httplib::Server>();
        start_time = std::chrono::steady_clock::now();
        setupRoutes();
        setupMiddleware();
    }

    void setupMiddleware() {
        // CORS middleware
        server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return httplib::Server::HandlerResponse::Unhandled;
        });

        // Logging middleware
        server->set_logger([](const httplib::Request& req, const httplib::Response& res) {
            auto now = system_clock::now();
            auto time_t = system_clock::to_time_t(now);
            // Use chrono formatting with std::format (C++20)
            std::cout << std::format("[{:%Y-%m-%d %H:%M:%S}] {} {} - Status: {}\n",
                                   std::chrono::floor<std::chrono::seconds>(now),
                                   req.method, req.path, res.status);
        });
    }

    void setupRoutes() {
        // Health check endpoint
        server->Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
            handleHealthCheck(req, res);
        });

        // Formatted text endpoint for browser display
        server->Get("/display", [this](const httplib::Request& req, httplib::Response& res) {
            handleDisplayText(req, res);
        });

        // API info endpoint
        server->Get("/api/info", [this](const httplib::Request& req, httplib::Response& res) {
            handleApiInfo(req, res);
        });

        // Root endpoint redirect
        server->Get("/", [](const httplib::Request& req, httplib::Response& res) {
            res.set_redirect("/display");
        });

        // Handle 404
        server->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            if (res.status == 404) {
                res.set_content(R"(
                    <!DOCTYPE html>
                    <html>
                    <head><title>404 - Not Found</title></head>
                    <body style="font-family: Arial, sans-serif; text-align: center; margin-top: 100px;">
                        <h1>404 - Page Not Found</h1>
                        <p>The requested resource was not found on this server.</p>
                        <a href="/display">Go to main page</a>
                    </body>
                    </html>
                )", "text/html");
            }
        });
    }

private:
    void handleHealthCheck(const httplib::Request& req, httplib::Response& res) {
        auto now = steady_clock::now();
        auto uptime_seconds = duration_cast<seconds>(now - start_time).count();
        
        json health_response = {
            {"status", "healthy"},
            {"service", "cpp-web-service"},
            {"version", version},
            {"timestamp", system_clock::to_time_t(system_clock::now())},
            {"uptime_seconds", uptime_seconds},
            {"port", port}
        };

        res.set_content(health_response.dump(2), "application/json");
        res.status = 200;
    }

    void handleDisplayText(const httplib::Request& req, httplib::Response& res) {
        // Get optional query parameters
        std::string name = req.get_param_value("name");
        std::string theme = req.get_param_value("theme");
        
        if (name.empty()) name = "World";
        if (theme.empty()) theme = "light";

        std::string background_color = (theme == "dark") ? "#1a1a1a" : "#f5f5f5";
        std::string text_color = (theme == "dark") ? "#ffffff" : "#333333";
        std::string accent_color = (theme == "dark") ? "#4a9eff" : "#007bff";

        // Get current time point
        auto now = std::chrono::system_clock::now();

        // Format the time directly using std::chrono::format
        // Note: std::chrono::current_zone() for local time zone.
        // If you specifically need UTC, you can use std::chrono::utc_clock::now()
        // and format it directly.
        std::string formatted_time = std::format("{:%Y-%m-%d %H:%M:%S %Z}", now);
        
        std::string html_content = std::format(R"(
            <!DOCTYPE html>
            <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>C++ Web Service</title>
                <style>
                    body {{
                        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                        background-color: {};
                        color: {};
                        margin: 0;
                        padding: 40px;
                        line-height: 1.6;
                    }}
                    .container {{
                        max-width: 800px;
                        margin: 0 auto;
                        text-align: center;
                    }}
                    h1 {{
                        color: {};
                        font-size: 3em;
                        margin-bottom: 20px;
                        text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
                    }}
                    .info-box {{
                        background: rgba(74, 158, 255, 0.1);
                        border: 2px solid {};
                        border-radius: 15px;
                        padding: 30px;
                        margin: 30px 0;
                    }}
                    .timestamp {{
                        font-size: 1.2em;
                        margin: 20px 0;
                        opacity: 0.8;
                    }}
                    .features {{
                        display: grid;
                        grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                        gap: 20px;
                        margin: 30px 0;
                    }}
                    .feature {{
                        background: rgba(255,255,255,0.05);
                        padding: 20px;
                        border-radius: 10px;
                        border: 1px solid rgba(255,255,255,0.1);
                    }}
                    .links {{
                        margin-top: 40px;
                    }}
                    .links a {{
                        color: {};
                        text-decoration: none;
                        margin: 0 15px;
                        padding: 10px 20px;
                        border: 2px solid {};
                        border-radius: 25px;
                        transition: all 0.3s ease;
                    }}
                    .links a:hover {{
                        background-color: {};
                        color: white;
                    }}
                </style>
            </head>
            <body>
                <div class="container">
                    <h1>🚀 Hello, {}!</h1>
                    
                    <div class="info-box">
                        <h2>Welcome to Modern C++ Web Service</h2>
                        <p>This service is built with C++20 and cpp-httplib, deployed on AWS.</p>
                        <div class="timestamp">
                            Current server time: {}
                        </div>
                    </div>

                    <div class="features">
                        <div class="feature">
                            <h3>⚡ High Performance</h3>
                            <p>Built with modern C++20 features</p>
                        </div>
                        <div class="feature">
                            <h3>🔧 RESTful API</h3>
                            <p>Clean and scalable endpoints</p>
                        </div>
                        <div class="feature">
                            <h3>☁️ Cloud Ready</h3>
                            <p>AWS deployment configured</p>
                        </div>
                        <div class="feature">
                            <h3>📊 Health Monitoring</h3>
                            <p>Built-in health checks</p>
                        </div>
                    </div>

                    <div class="links">
                        <a href="/health">Health Check</a>
                        <a href="/api/info">API Info</a>
                        <a href="/display?theme={}">Switch Theme</a>
                    </div>
                </div>
            </body>
            </html>
        )", 
        background_color, text_color, accent_color, accent_color, accent_color, accent_color, accent_color, name,
        formatted_time,
        (theme == "dark") ? "light" : "dark");

        res.set_content(html_content, "text/html");
        res.status = 200;
    }

    void handleApiInfo(const httplib::Request& req, httplib::Response& res) {
        json api_info = {
            {"service_name", "cpp-web-service"},
            {"version", version},
            {"description", "Modern C++ Web Service with C++20 features"},
            {"endpoints", {
                {"/health", "GET - Health check endpoint"},
                {"/display", "GET - Formatted text display (supports ?name= and ?theme= params)"},
                {"/api/info", "GET - API information"},
                {"/", "GET - Redirects to /display"}
            }},
            {"features", {
                "C++20 compatibility",
                "AWS deployment ready",
                "CORS enabled",
                "Request logging",
                "JSON responses",
                "HTML rendering"
            }}
        };

        res.set_content(api_info.dump(2), "application/json");
        res.status = 200;
    }

public:
    void start() {
        std::cout << std::format("🚀 Starting C++ Web Service v{}\n", version);
        std::cout << std::format("📡 Server listening on port {}\n", port);
        std::cout << std::format("🌐 Access the service at http://localhost:{}\n", port);
        std::cout << "📋 Available endpoints:\n";
        std::cout << "   GET /health   - Health check\n";
        std::cout << "   GET /display  - Formatted text display\n";
        std::cout << "   GET /api/info - API information\n";
        std::cout << "Press Ctrl+C to stop the server.\n\n";

        if (!server->listen("0.0.0.0", port)) {
            std::cerr << "❌ Failed to start server on port " << port << std::endl;
            exit(1);
        }
    }

    void stop() {
        if (server) {
            server->stop();
            std::cout << "\n🛑 Server stopped gracefully.\n";
        }
    }
};

// Signal handling for graceful shutdown
#include <signal.h>
std::unique_ptr<WebService> global_service;

void signalHandler(int signum) {
    std::cout << "\n📡 Received shutdown signal (" << signum << ")\n";
    if (global_service) {
        global_service->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    // Handle command line arguments for port
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
            if (port < 1024 || port > 65535) {
                std::cerr << "❌ Port must be between 1024 and 65535\n";
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ Invalid port number: " << argv[1] << std::endl;
            return 1;
        }
    }

    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        global_service = std::make_unique<WebService>(port);
        global_service->start();
    } catch (const std::exception& e) {
        std::cerr << "❌ Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}