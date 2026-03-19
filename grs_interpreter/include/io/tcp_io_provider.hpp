#ifndef TCP_IO_PROVIDER_HPP_
#define TCP_IO_PROVIDER_HPP_

#include "io/io_provider.hpp"
#include <string>
#include <cstdint>
#include <cstring>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// ═══════════════════════════════════════════════════════════════
// Legacy Protocol — EtherCAT I/O only (EL1008 + EL2008)
// ═══════════════════════════════════════════════════════════════
#pragma pack(push, 1)
struct EcrtRobotState {
    uint64_t seq_id;
    uint32_t timestamp;
    uint8_t  inputs;
    uint8_t  outputs;
    uint8_t  is_hardware_emg;
    uint8_t  system_ready;
    uint8_t  padding[2];
};
// sizeof = 18

struct EcrtRobotCommand {
    uint64_t cmd_id;
    uint8_t  set_outputs;
    uint8_t  soft_stops;
    uint8_t  padding[6];
};
// sizeof = 16

// ═══════════════════════════════════════════════════════════════
// Extended Protocol — Full Robot Command (Motion + I/O + Wait)
// ═══════════════════════════════════════════════════════════════

enum GrsCommandType : uint8_t {
    GRS_CMD_NOP        = 0,
    GRS_CMD_PTP        = 1,
    GRS_CMD_PTP_REL    = 2,
    GRS_CMD_LIN        = 3,
    GRS_CMD_LIN_REL    = 4,
    GRS_CMD_CIRC       = 5,
    GRS_CMD_CIRC_REL   = 6,
    GRS_CMD_SPLINE     = 7,
    GRS_CMD_SPLINE_REL = 8,
    GRS_CMD_WAIT       = 9,
    GRS_CMD_OUTPUT     = 10,
    GRS_CMD_SET_ALL_OUTPUTS = 11,
};

// GRS Interpreter -> Hardware Controller (128 bytes)
struct GrsRobotCommand {
    uint64_t cmd_id;           // 8
    uint8_t  cmd_type;         // 1  - GrsCommandType
    uint8_t  io_index;         // 1  - I/O bit index (0-based)
    uint8_t  io_value;         // 1  - I/O value (0 or 1)
    uint8_t  set_outputs;      // 1  - full output byte
    uint8_t  soft_stops;       // 1  - soft emergency stop
    uint8_t  reserved[3];      // 3
    double   wait_time;        // 8  - ms (for WAIT)
    double   coords[6];        // 48 - x,y,z,a,b,c
    double   axes[6];          // 48 - A1-A6
    uint8_t  padding[8];       // 8
};
// sizeof = 128

// Hardware Controller -> GRS Interpreter (128 bytes)
struct GrsRobotState {
    uint64_t seq_id;           // 8
    uint32_t timestamp;        // 4
    uint8_t  inputs;           // 1
    uint8_t  outputs;          // 1
    uint8_t  is_hardware_emg;  // 1
    uint8_t  system_ready;     // 1
    uint8_t  cmd_ack;          // 1  - last ack'd cmd_type
    uint8_t  cmd_status;       // 1  - 0=idle,1=exec,2=done,3=err
    uint8_t  reserved[2];      // 2
    double   current_pos[6];   // 48
    double   current_axes[6];  // 48
    uint8_t  padding[12];      // 12
};
// sizeof = 128

#pragma pack(pop)

namespace grs_io {

class TcpIOProvider : public IOProvider {
public:
    TcpIOProvider(const std::string& host = "127.0.0.1", int port = 12345,
                  bool extendedProtocol = false);
    ~TcpIOProvider();

    bool connect();
    void disconnect();
    bool isConnected() const { return connected_; }

    // IOProvider interface
    bool readDigitalInput(uint8_t index) override;
    void writeDigitalOutput(uint8_t index, bool value) override;
    bool readDigitalOutput(uint8_t index) override;

    // Extended protocol: send full robot command
    bool sendRobotCommand(uint8_t cmdType,
                          const double coords[6], const double axes[6],
                          double waitTime = 0.0,
                          uint8_t ioIndex = 0, uint8_t ioValue = 0);

    // State info (works for both legacy and extended)
    uint8_t getInputByte() const;
    uint8_t getOutputByte() const;
    bool isSystemReady() const;
    bool isHardwareEmg() const;

    // Extended state info
    bool isExtended() const { return extendedProtocol_; }
    void getCurrentPosition(double pos[6]) const;
    void getCurrentAxes(double axes[6]) const;
    uint8_t getCommandStatus() const;

private:
    std::string host_;
    int port_;
    int socket_fd_ = -1;
    bool connected_ = false;
    bool extendedProtocol_;   // true = 128-byte protocol, false = legacy 16/18

    // Background thread receives state updates
    std::atomic<bool> running_{false};
    std::thread recvThread_;
    mutable std::mutex stateMutex_;

    // Cached state (both protocols share I/O fields)
    EcrtRobotState legacyState_{};
    GrsRobotState  extendedState_{};

    // Command tracking
    uint64_t cmdIdCounter_ = 1;

    void recvLoop();
    bool sendLegacyCommand(uint8_t outputByte, bool softStop = false);
};

} // namespace grs_io

#endif // TCP_IO_PROVIDER_HPP_
