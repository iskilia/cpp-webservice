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
        
        if (name.empty()) name = "Dad";
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
                        padding: 20px;
                        line-height: 1.6;
						transition: background-color 0.3s ease, color 0.3s ease;
                    }}
                    .container {{
                        max-width: 800px;
                        margin: 0 auto;
                        text-align: center;
						padding: 0 10px; /* Add some horizontal padding for very small screens */
                    }}
                    h1 {{
                        color: {};
                        font-size: 2.5em;
                        margin-bottom: 20px;
                        text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
                    }}
                    .info-box {{
                        background: rgba(74, 158, 255, 0.1);
                        border: 2px solid {};
                        border-radius: 15px;
                        padding: 20px;
                        margin: 20px 0;
                    }}
                    .timestamp {{
                        font-size: 1em;
                        margin: 15px 0;
                        opacity: 0.8;
                    }}
                    .features {{
                        display: grid;
                        grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                        gap: 15px;
                        margin: 20px 0;
                    }}
                    .feature {{
                        background: rgba(255,255,255,0.05);
                        padding: 15px;
                        border-radius: 10px;
                        border: 1px solid rgba(255,255,255,0.1);
						font-size: 0.9em;
                    }}
                    .links {{
                        margin-top: 30px;
            			display: flex;
            			flex-wrap: wrap;
            			justify-content: center;
            			gap: 15px;
                    }}
                    .links a {{
                        color: {};
                        text-decoration: none;
                        padding: 10px 15px;
                        border: 2px solid {};
                        border-radius: 25px;
                        transition: all 0.3s ease;
                        white-space: nowrap;
            			font-size: 0.9em;
                    }}
                    .links a:hover {{
                        background-color: {};
                        color: white;
                    }}

        			@media (max-width: 768px) {{
            			body {{
                			padding: 15px;
            			}}
            			h1 {{
                			font-size: 2em;
            			}}
            			.info-box h2 {{
                			font-size: 1.5em;
            			}}
            			.feature h3 {{
                			font-size: 1.1em;
            			}}
        			}}

        			@media (max-width: 480px) {{
            			body {{
                			padding: 10px;
            			}}
            			h1 {{
                			font-size: 1.8em;
            			}}
						.info-box, .features, .links {{
                			margin: 15px 0;
            			}}
            			.info-box h2 {{
                			font-size: 1.3em;
            			}}
            			.feature {{
                			padding: 10px;
							font-size: 0.85em;
            			}}
						.links a {{
							padding: 8px 12px;
							font-size: 0.8em;
						}}
        			}}
                </style>
            </head>
            <body>
                <div class="container">
                    <h1>🚀 Hello {}!</h1>

                    <div class="info-box">
                        <h2>Happy Father's Day Dad!</h2>
                        <p >I just wanted to say thank you for all the guidance you've given me over the years. Even if I sometimes seem stubborn or distracted, please know that I'm always listening (eventually), and I'm forever grateful for your wisdom and support. Also… yes, you’re reading this from a C++ web service deployed on AWS. What can I say? I’ve been brushing up on my C++ skills lately as per your guidance. I know, I know—using C++ for a web service is an overengineered solution. But I thought it would a good learning experience for me. I’m lucky to have you not just as my father, but as someone I look up to career wise as well! I hope this message finds you well, and I really hope we can meet up again in person this year—maybe in December when I am back in Asia</p>
						<p>With love (and a few dangling pointers), </p>
                        <p>Isaac</p>
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