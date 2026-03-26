#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <bitset>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "executor/step_executor.hpp"
#include "io/io_provider.hpp"
#include "io/tcp_io_provider.hpp"
#include "common/utils.hpp"

namespace fs = std::filesystem;

// Global pointer for signal handler cleanup
static std::shared_ptr<grs_io::TcpIOProvider> g_tcpIO;
static std::atomic<bool> g_terminated{false};

static void signalHandler(int sig) {
    g_terminated = true;
    // Clean shutdown: clear outputs and disconnect TCP
    if (g_tcpIO) {
        for (int i = 0; i < 8; i++) {
            g_tcpIO->writeDigitalOutput(i, false);
        }
        // Small delay so the clear-outputs packet reaches Holly
        usleep(50000);  // 50ms
        g_tcpIO->disconnect();
        g_tcpIO.reset();
    }
    std::_Exit(0);
}

void printRobotCommand(const grs_executor::RobotCommand& cmd) {
    static const char* typeNames[] = {
        "PTP", "PTP_REL", "LIN", "LIN_REL", "CIRC", "CIRC_REL",
        "SPLINE", "SPLINE_REL", "WAIT", "OUTPUT", "UNKNOWN"
    };
    
    int idx = static_cast<int>(cmd.type);
    std::cout << "  [ROBOT CMD] " << typeNames[idx];
    
    if (cmd.type == grs_executor::RobotCommand::Type::OUTPUT) {
        std::cout << " $OUT[" << (int)cmd.ioIndex << "] = " << (cmd.ioValue ? "TRUE" : "FALSE");
    } else if (cmd.type == grs_executor::RobotCommand::Type::WAIT) {
        std::cout << " time=" << cmd.waitTime;
    } else {
        std::cout << " target=" << cmd.targetName;
        // Output params in canonical order: x,y,z,a,b,c then a1-a6/A1-A6
        static const std::string orderedKeys[] = {
            "x","y","z","a","b","c",
            "a1","a2","a3","a4","a5","a6",
            "A1","A2","A3","A4","A5","A6"
        };
        // First: known keys in order
        for (const auto& ok : orderedKeys) {
            for (const auto& [k, v] : cmd.params) {
                if (k == ok) {
                    std::cout << " " << k << "=" << common::valueToString(v);
                    break;
                }
            }
        }
        // Then: any remaining keys not in ordered list
        for (const auto& [k, v] : cmd.params) {
            bool found = false;
            for (const auto& ok : orderedKeys) {
                if (k == ok) { found = true; break; }
            }
            if (!found) {
                std::cout << " " << k << "=" << common::valueToString(v);
            }
        }
    }
    std::cout << " (line " << cmd.sourceLine << ")" << std::endl;
}

// Helper: send a RobotCommand to hardware via TCP
// Used by all modes (debug, step, run) when --tcp is active
void sendTcpCommand(const std::shared_ptr<grs_io::TcpIOProvider>& tcpIO,
                            const grs_executor::RobotCommand& cmd) {
    if (!tcpIO) return;
    // OUTPUT is already sent by writeDigitalOutput() in the executor
    if (cmd.type == grs_executor::RobotCommand::Type::OUTPUT) return;

    static const uint8_t typeMap[] = {
        GRS_CMD_PTP, GRS_CMD_PTP_REL, GRS_CMD_LIN, GRS_CMD_LIN_REL,
        GRS_CMD_CIRC, GRS_CMD_CIRC_REL, GRS_CMD_SPLINE, GRS_CMD_SPLINE_REL,
        GRS_CMD_WAIT, GRS_CMD_OUTPUT, GRS_CMD_NOP
    };
    uint8_t cmdType = typeMap[static_cast<int>(cmd.type)];

    double coords[6] = {};
    double axes[6] = {};
    static const char* coordNames[] = {"x","y","z","a","b","c"};
    static const char* axisNames[] = {"A1","A2","A3","A4","A5","A6"};

    for (const auto& [key, val] : cmd.params) {
        auto toDouble = [](const common::ValueType& v) -> double {
            if (auto* d = std::get_if<double>(&v)) return *d;
            if (auto* i = std::get_if<int>(&v)) return *i;
            return 0.0;
        };
        std::string lkey = key;
        for (auto& ch : lkey) ch = std::tolower(ch);
        for (int i = 0; i < 6; i++) {
            if (lkey == coordNames[i]) { coords[i] = toDouble(val); break; }
        }
        std::string ukey = key;
        for (auto& ch : ukey) ch = std::toupper(ch);
        for (int i = 0; i < 6; i++) {
            if (ukey == axisNames[i]) { axes[i] = toDouble(val); break; }
        }
    }

    tcpIO->sendRobotCommand(cmdType, coords, axes, cmd.waitTime, 0, 0);
}

int main(int argc, char* argv[]) {
    fs::path testFile;
    bool stepMode = false;
    bool debugMode = false;   // --debug: JSON-line protocol for IDE
    std::string tcpHost = "";
    int tcpPort = 12345;

    // Argumentları parse et
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--step" || arg == "-s") {
            stepMode = true;
        } else if (arg == "--debug" || arg == "-d") {
            debugMode = true;
        } else if (arg == "--tcp") {
            if (i + 1 < argc && argv[i+1][0] != '-') {
                tcpHost = argv[++i];
                auto colon = tcpHost.find(':');
                if (colon != std::string::npos) {
                    tcpPort = std::stoi(tcpHost.substr(colon + 1));
                    tcpHost = tcpHost.substr(0, colon);
                }
            } else {
                tcpHost = "127.0.0.1";
            }
        } else {
            testFile = arg;
        }
    }

    if (testFile.empty()) {
        testFile = "../tests/parser_test.txt";
    }

    // Dosyayı oku
    std::ifstream file(testFile);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << testFile << std::endl;
        return 1;
    }
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();

    // Suppress parser/lexer debug noise (Token matched, expression() called, etc.)
    // These go to cerr so they don't flood ZeroBrane output or mix with JSON protocol
    std::streambuf* origCoutBuf = std::cout.rdbuf();
    std::cout.rdbuf(std::cerr.rdbuf());

    // Lexer
    grs_lexer::Lexer lexer;
    auto tokens = lexer.tokenize(code);

    // Parser
    grs_parser::Parser parser;
    auto ast = parser.parse(tokens);

    // Restore stdout for actual program output
    std::cout.rdbuf(origCoutBuf);

    if (!debugMode) {
        std::cout << "=== GRS Step Executor ===" << std::endl;
        std::cout << "File: " << testFile << std::endl;
        std::cout << "Mode: " << (stepMode ? "STEP" : "RUN") << std::endl;
        std::cout << "=========================" << std::endl;
    }

    if (parser.hasErrors()) {
        for (const auto& error : parser.getErrors()) {
            std::cerr << "Parse Error: " << error.message 
                      << " (Line: " << error.line << ")" << std::endl;
        }
        return 1;
    }

    if (!ast) {
        std::cerr << "AST generation failed!" << std::endl;
        return 1;
    }

    // I/O Provider seçimi
    std::shared_ptr<grs_io::IOProvider> ioProvider;
    std::shared_ptr<grs_io::LocalIOProvider> localIO;
    std::shared_ptr<grs_io::TcpIOProvider> tcpIO;

    if (!tcpHost.empty()) {
        tcpIO = std::make_shared<grs_io::TcpIOProvider>(tcpHost, tcpPort);
        if (tcpIO->connect()) {
            ioProvider = tcpIO;
            if (!debugMode) {
                std::cout << "[TCP] Connected to robot at " << tcpHost << ":" << tcpPort
                          << std::endl;
            }
        } else {
            // Fallback to local I/O — continue without hardware
            std::cerr << "[TCP] Connection failed to " << tcpHost << ":" << tcpPort
                      << " — running in offline mode" << std::endl;
            if (!debugMode) {
                std::cout << "[TCP] Offline mode — no hardware connection" << std::endl;
            }
            tcpIO.reset();
            localIO = std::make_shared<grs_io::LocalIOProvider>();
            localIO->setDigitalInput(1, false);
            ioProvider = localIO;
        }
    } else {
        localIO = std::make_shared<grs_io::LocalIOProvider>();
        localIO->setDigitalInput(1, false);
        ioProvider = localIO;
    }

    // Step Executor
    grs_executor::StepExecutor executor(ioProvider);

    // Register signal handler for clean TCP shutdown on kill
    if (tcpIO) {
        g_tcpIO = tcpIO;
        std::signal(SIGTERM, signalHandler);
        std::signal(SIGINT, signalHandler);
    }

    executor.load(ast);

    // ═══════════════════════════════════════════════════════════
    // DEBUG MODE — JSON-line protocol over stdin/stdout
    // ZeroBrane Studio bunu kullanarak debug yapabilir
    // Protocol:
    //   IDE → grs_step (stdin):  {"cmd":"step"} | {"cmd":"continue"} | {"cmd":"setBreakpoint","line":5}
    //                            {"cmd":"getVariables"} | {"cmd":"getIO"} | {"cmd":"disconnect"}
    //   grs_step → IDE (stdout): {"event":"stopped","line":3,"reason":"step"}
    //                            {"event":"output","type":"PTP","target":"P1","line":5}
    //                            {"event":"variables","data":[{"name":"x","value":"5","type":"INT"}]}
    //                            {"event":"terminated"}
    // ═══════════════════════════════════════════════════════════
    if (debugMode) {
        // Robot command callback — JSON output event + TCP extended send
        executor.setCommandCallback([&executor, &tcpIO](const grs_executor::RobotCommand& cmd) {
            static const char* typeNames[] = {
                "PTP","PTP_REL","LIN","LIN_REL","CIRC","CIRC_REL",
                "SPLINE","SPLINE_REL","WAIT","OUTPUT","UNKNOWN"
            };
            int idx = static_cast<int>(cmd.type);
            std::cout << "{\"event\":\"output\",\"type\":\"" << typeNames[idx] << "\"";
            if (cmd.type == grs_executor::RobotCommand::Type::OUTPUT) {
                std::cout << ",\"index\":" << (int)cmd.ioIndex 
                          << ",\"value\":" << (cmd.ioValue ? "true" : "false");
            } else if (cmd.type == grs_executor::RobotCommand::Type::WAIT) {
                std::cout << ",\"time\":" << cmd.waitTime;
            } else {
                std::cout << ",\"target\":\"" << cmd.targetName << "\"";
                // Include position parameters in x,y,z,a,b,c order for IDE display
                if (!cmd.params.empty()) {
                    std::cout << ",\"params\":{";
                    // cmd.params is std::vector<std::pair<string, ValueType>>
                    // Output in canonical order: x,y,z,a,b,c then a1-a6/A1-A6
                    static const std::string orderedKeys[] = {
                        "x","y","z","a","b","c",
                        "a1","a2","a3","a4","a5","a6",
                        "A1","A2","A3","A4","A5","A6"
                    };
                    bool first = true;
                    // First: known keys in order
                    for (const auto& ok : orderedKeys) {
                        for (const auto& [k, v] : cmd.params) {
                            if (k == ok) {
                                if (!first) std::cout << ",";
                                first = false;
                                std::cout << "\"" << k << "\":" << common::valueToString(v);
                                break;
                            }
                        }
                    }
                    // Then: any remaining keys not in ordered list
                    for (const auto& [k, v] : cmd.params) {
                        bool found = false;
                        for (const auto& ok : orderedKeys) {
                            if (k == ok) { found = true; break; }
                        }
                        if (!found) {
                            if (!first) std::cout << ",";
                            first = false;
                            std::cout << "\"" << k << "\":" << common::valueToString(v);
                        }
                    }
                    std::cout << "}";
                }
            }
            std::cout << ",\"line\":" << cmd.sourceLine << "}" << std::endl;
            
            // Send motion/wait commands to hardware via TCP
            sendTcpCommand(tcpIO, cmd);
            
            // Auto-ACK in debug mode
            executor.acknowledgeCommand();
        });

        // Send initial ready event (include first line number for IDE cursor positioning)
        std::cout << "{\"event\":\"initialized\",\"line\":" << executor.getCurrentLine() << "}" << std::endl;

        std::string line;
        while (std::getline(std::cin, line)) {
            // Simple JSON parsing (no external library needed)
            // Extract "cmd" field value
            std::string cmd;
            auto cmdPos = line.find("\"cmd\"");
            if (cmdPos != std::string::npos) {
                auto colonPos = line.find(':', cmdPos);
                auto q1 = line.find('"', colonPos + 1);
                auto q2 = line.find('"', q1 + 1);
                if (q1 != std::string::npos && q2 != std::string::npos) {
                    cmd = line.substr(q1 + 1, q2 - q1 - 1);
                }
            }

            if (cmd == "step") {
                if (executor.getStatus() == grs_executor::ExecutionStatus::WAITING_ACK) {
                    executor.acknowledgeCommand();
                }
                bool hasMore = executor.step();
                if (executor.getStatus() == grs_executor::ExecutionStatus::ERROR) {
                    std::cout << "{\"event\":\"error\",\"message\":\"" 
                              << executor.getErrorMessage() << "\"}" << std::endl;
                } else if (!hasMore || executor.getStatus() == grs_executor::ExecutionStatus::COMPLETED) {
                    std::cout << "{\"event\":\"terminated\"}" << std::endl;
                    break;
                } else {
                    std::cout << "{\"event\":\"stopped\",\"line\":" << executor.getCurrentLine() 
                              << ",\"reason\":\"step\"}" << std::endl;
                }
            }
            else if (cmd == "continue") {
                if (executor.getStatus() == grs_executor::ExecutionStatus::WAITING_ACK) {
                    executor.acknowledgeCommand();
                }
                executor.run();
                if (executor.getStatus() == grs_executor::ExecutionStatus::COMPLETED) {
                    std::cout << "{\"event\":\"terminated\"}" << std::endl;
                    break;
                } else if (executor.getStatus() == grs_executor::ExecutionStatus::ERROR) {
                    std::cout << "{\"event\":\"error\",\"message\":\"" 
                              << executor.getErrorMessage() << "\"}" << std::endl;
                } else {
                    std::cout << "{\"event\":\"stopped\",\"line\":" << executor.getCurrentLine() 
                              << ",\"reason\":\"breakpoint\"}" << std::endl;
                }
            }
            else if (cmd == "setBreakpoint") {
                // Extract "line" field
                auto linePos = line.find("\"line\"");
                if (linePos != std::string::npos) {
                    auto colonPos = line.find(':', linePos);
                    int bpLine = std::stoi(line.substr(colonPos + 1));
                    executor.addBreakpoint(bpLine);
                    std::cout << "{\"event\":\"breakpointSet\",\"line\":" << bpLine << "}" << std::endl;
                }
            }
            else if (cmd == "removeBreakpoint") {
                auto linePos = line.find("\"line\"");
                if (linePos != std::string::npos) {
                    auto colonPos = line.find(':', linePos);
                    int bpLine = std::stoi(line.substr(colonPos + 1));
                    executor.removeBreaPoint(bpLine);
                    std::cout << "{\"event\":\"breakpointRemoved\",\"line\":" << bpLine << "}" << std::endl;
                }
            }
            else if (cmd == "clearBreakpoints") {
                executor.clearBreakPoint();
                std::cout << "{\"event\":\"breakpointsCleared\"}" << std::endl;
            }
            else if (cmd == "getVariables") {
                std::cout << "{\"event\":\"variables\",\"data\":[";
                bool first = true;
                for (const auto& [name, value] : executor.getVariables()) {
                    if (!first) std::cout << ",";
                    first = false;
                    std::cout << "{\"name\":\"" << name 
                              << "\",\"value\":\"" << common::valueToString(value) << "\"}";
                }
                std::cout << "]}" << std::endl;
            }
            else if (cmd == "getIO") {
                if (localIO) {
                    std::cout << "{\"event\":\"io\",\"inputs\":" << localIO->getInputWord()
                              << ",\"outputs\":" << localIO->getOutputWord() << "}" << std::endl;
                } else if (tcpIO) {
                    std::cout << "{\"event\":\"io\",\"inputs\":" << (int)tcpIO->getInputByte()
                              << ",\"outputs\":" << (int)tcpIO->getOutputByte() 
                              << ",\"ready\":" << (tcpIO->isSystemReady() ? "true" : "false")
                              << "}" << std::endl;
                }
            }
            else if (cmd == "disconnect") {
                executor.stop();
                std::cout << "{\"event\":\"terminated\"}" << std::endl;
                break;
            }
        }

        if (tcpIO) tcpIO->disconnect();
        return 0;
    }

    // ═══════════════════════════════════════════════════════════
    // Normal modes: --step (interactive) or run (batch)
    // ═══════════════════════════════════════════════════════════
    
    // Robot command callback (step mode — run mode overrides this later)
    executor.setCommandCallback([&tcpIO](const grs_executor::RobotCommand& cmd) {
        printRobotCommand(cmd);
        // Send motion/wait commands to hardware via TCP
        sendTcpCommand(tcpIO, cmd);
    });

    // Status callback
    // Status callback — mod'a göre farklı davranış
    executor.setStatusCallback([stepMode](grs_executor::ExecutionStatus status, int line) {
        static const char* statusNames[] = {
            "IDLE", "RUNNING", "PAUSED", "WAITING_ACK", "COMPLETED", "ERROR"
        };
        if (stepMode) {
            // Step modda her status'u göster (RUNNING hariç)
            if (status != grs_executor::ExecutionStatus::RUNNING) {
                std::cout << "  [STATUS] " << statusNames[static_cast<int>(status)]
                          << " at line " << line << std::endl;
            }
        } else {
            // RUN modda sadece COMPLETED ve ERROR göster
            if (status == grs_executor::ExecutionStatus::COMPLETED ||
                status == grs_executor::ExecutionStatus::ERROR) {
                std::cout << "  [STATUS] " << statusNames[static_cast<int>(status)]
                          << " at line " << line << std::endl;
            }
        }
    });

    // Helper: print I/O state
    auto printIOState = [&]() {
        if (tcpIO) {
            // Short sleep to let recvLoop pick up the updated state from hardware
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::cout << "  [I/O] IN: " << std::bitset<8>(tcpIO->getInputByte())
                      << " | OUT: " << std::bitset<8>(tcpIO->getOutputByte())
                      << " | Ready: " << (tcpIO->isSystemReady() ? "YES" : "NO")
                      << std::endl;
        } else if (localIO) {
            std::cout << "  [I/O] IN: " << std::bitset<8>(localIO->getInputWord() & 0xFF)
                      << " | OUT: " << std::bitset<8>(localIO->getOutputWord() & 0xFF)
                      << std::endl;
        }
    };

    if (stepMode) {
        // ─── Interactive Step Mode ───
        std::cout << "\nCommands: [Enter]=step, r=run, b <line>=breakpoint, "
                  << "v=variables, io=show I/O, q=quit" << std::endl;
        if (tcpIO) {
            std::cout << "[TCP] Hardware mode — I/O changes are sent to robot" << std::endl;
            printIOState();
        }
        std::cout << std::endl;
        
        std::string input;
        while (executor.getStatus() != grs_executor::ExecutionStatus::COMPLETED &&
               executor.getStatus() != grs_executor::ExecutionStatus::ERROR) {
            
            std::cout << "[line " << executor.getCurrentLine() << "] > ";
            std::getline(std::cin, input);

            if (input.empty()) {
                // Step
                if (executor.getStatus() == grs_executor::ExecutionStatus::WAITING_ACK) {
                    executor.acknowledgeCommand();
                }
                executor.step();
                // Show I/O state after each step (especially useful for TCP hardware)
                printIOState();
            } 
            else if (input == "r") {
                if (executor.getStatus() == grs_executor::ExecutionStatus::WAITING_ACK) {
                    executor.acknowledgeCommand();
                }
                executor.run();
                printIOState();
            }
            else if (input.substr(0, 2) == "b ") {
                int bpLine = std::stoi(input.substr(2));
                executor.addBreakpoint(bpLine);
                std::cout << "  Breakpoint set at line " << bpLine << std::endl;
            }
            else if (input == "v") {
                std::cout << "  Variables:" << std::endl;
                for (const auto& [name, value] : executor.getVariables()) {
                    std::cout << "    " << name << " = " << common::valueToString(value) << std::endl;
                }
            }
            else if (input == "io") {
                printIOState();
            }
            else if (input == "q") {
                // TCP modda çıkışları temizle
                if (tcpIO) {
                    std::cout << "  [TCP] Clearing outputs..." << std::endl;
                    for (int i = 0; i < 8; i++) {
                        tcpIO->writeDigitalOutput(i, false);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    printIOState();
                }
                executor.stop();
                break;
            }
        }

        if (executor.getStatus() == grs_executor::ExecutionStatus::ERROR) {
            std::cerr << "Error: " << executor.getErrorMessage() << std::endl;
        }
    } 
    else {
        // ─── Run mode — auto ACK, continuous execution ───
        if (tcpIO) {
            std::cout << "[TCP] Hardware mode — running program on robot" << std::endl;
            printIOState();
            std::cout << std::endl;
        }

        executor.setCommandCallback([&executor, &printIOState, &tcpIO](const grs_executor::RobotCommand& cmd) {
            printRobotCommand(cmd);

            // Send motion/wait commands to hardware via TCP
            sendTcpCommand(tcpIO, cmd);

            // WAIT komutu: gerçek bekleme yap
            if (cmd.type == grs_executor::RobotCommand::Type::WAIT) {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(cmd.waitTime)));
            }

            // OUTPUT komutu: kısa bekleme + durum göster
            if (cmd.type == grs_executor::RobotCommand::Type::OUTPUT && tcpIO) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                printIOState();
            }

            executor.acknowledgeCommand();
        });
        
        executor.run();

        // TCP modda çıkışta output'ları temizle
        if (tcpIO) {
            std::cout << "\n  [TCP] Clearing outputs..." << std::endl;
            for (int i = 0; i < 8; i++) {
                tcpIO->writeDigitalOutput(i, false);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            printIOState();
        }
    }

    std::cout << "\n=== Execution Complete ===" << std::endl;
    std::cout << "Final Variables:" << std::endl;
    for (const auto& [name, value] : executor.getVariables()) {
        std::cout << "  " << name << " = " << common::valueToString(value) << std::endl;
    }

    if (tcpIO) tcpIO->disconnect();

    return 0;
}
