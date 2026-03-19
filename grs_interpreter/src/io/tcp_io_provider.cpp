#include "io/tcp_io_provider.hpp"
#include <iostream>
#include <cstring>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <poll.h>
#include <cerrno>

namespace grs_io {

TcpIOProvider::TcpIOProvider(const std::string& host, int port, bool extendedProtocol)
    : host_(host), port_(port), extendedProtocol_(extendedProtocol) {
    std::memset(&legacyState_, 0, sizeof(legacyState_));
    std::memset(&extendedState_, 0, sizeof(extendedState_));
}

TcpIOProvider::~TcpIOProvider() {
    disconnect();
}

bool TcpIOProvider::connect() {
    socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        std::cerr << "[TCP] Socket creation failed" << std::endl;
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "[TCP] Invalid address: " << host_ << std::endl;
        ::close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    // Non-blocking connect with 2-second timeout
    // Prevents hanging when server is unreachable
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK);

    int ret = ::connect(socket_fd_, (sockaddr*)&addr, sizeof(addr));
    if (ret < 0 && errno != EINPROGRESS) {
        std::cerr << "[TCP] Connection failed to " << host_ << ":" << port_ << std::endl;
        ::close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    if (ret != 0) {
        // Wait for connection with timeout
        struct pollfd pfd;
        pfd.fd = socket_fd_;
        pfd.events = POLLOUT;
        int pollRet = poll(&pfd, 1, 2000); // 2 second timeout

        if (pollRet <= 0) {
            std::cerr << "[TCP] Connection timeout to " << host_ << ":" << port_ << std::endl;
            ::close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

        // Check for socket error
        int sockErr = 0;
        socklen_t errLen = sizeof(sockErr);
        getsockopt(socket_fd_, SOL_SOCKET, SO_ERROR, &sockErr, &errLen);
        if (sockErr != 0) {
            std::cerr << "[TCP] Connection refused by " << host_ << ":" << port_ << std::endl;
            ::close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }
    }

    // Restore blocking mode for normal I/O
    fcntl(socket_fd_, F_SETFL, flags);

    connected_ = true;
    running_ = true;

    // Start background receiver thread
    recvThread_ = std::thread(&TcpIOProvider::recvLoop, this);

    std::cerr << "[TCP] Connected to " << host_ << ":" << port_
              << " (" << (extendedProtocol_ ? "extended 128-byte" : "legacy 16/18-byte") << ")"
              << std::endl;
    return true;
}

void TcpIOProvider::disconnect() {
    running_ = false;
    if (recvThread_.joinable()) {
        recvThread_.join();
    }
    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
    connected_ = false;
}

void TcpIOProvider::recvLoop() {
    if (extendedProtocol_) {
        // Extended protocol: receive 128-byte GrsRobotState
        GrsRobotState state;
        while (running_ && connected_) {
            int n = ::recv(socket_fd_, &state, sizeof(state), MSG_WAITALL);
            if (n <= 0) {
                connected_ = false;
                running_ = false;
                break;
            }
            if (n == sizeof(state)) {
                std::lock_guard<std::mutex> lock(stateMutex_);
                extendedState_ = state;
            }
        }
    } else {
        // Legacy protocol: receive 18-byte EcrtRobotState
        EcrtRobotState state;
        while (running_ && connected_) {
            int n = ::recv(socket_fd_, &state, sizeof(state), 0);
            if (n <= 0) {
                connected_ = false;
                running_ = false;
                break;
            }
            if (n == sizeof(state)) {
                std::lock_guard<std::mutex> lock(stateMutex_);
                legacyState_ = state;
            }
        }
    }
}

// ─── IOProvider interface ───

bool TcpIOProvider::readDigitalInput(uint8_t index) {
    if (index >= 8) return false;
    std::lock_guard<std::mutex> lock(stateMutex_);
    uint8_t inputs = extendedProtocol_ ? extendedState_.inputs : legacyState_.inputs;
    return (inputs >> index) & 1;
}

void TcpIOProvider::writeDigitalOutput(uint8_t index, bool value) {
    if (index >= 8 || !connected_) return;

    // Read current output state, modify the bit, send
    uint8_t currentOutputs;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        currentOutputs = extendedProtocol_ ? extendedState_.outputs : legacyState_.outputs;
    }

    if (value)
        currentOutputs |= (1u << index);
    else
        currentOutputs &= ~(1u << index);

    if (extendedProtocol_) {
        // Use extended protocol for OUTPUT
        double zeroCoords[6] = {};
        double zeroAxes[6] = {};
        sendRobotCommand(GRS_CMD_OUTPUT, zeroCoords, zeroAxes, 0.0, index, value ? 1 : 0);
    } else {
        sendLegacyCommand(currentOutputs);
    }
}

bool TcpIOProvider::readDigitalOutput(uint8_t index) {
    if (index >= 8) return false;
    std::lock_guard<std::mutex> lock(stateMutex_);
    uint8_t outputs = extendedProtocol_ ? extendedState_.outputs : legacyState_.outputs;
    return (outputs >> index) & 1;
}

// ─── State info ───

uint8_t TcpIOProvider::getInputByte() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return extendedProtocol_ ? extendedState_.inputs : legacyState_.inputs;
}

uint8_t TcpIOProvider::getOutputByte() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return extendedProtocol_ ? extendedState_.outputs : legacyState_.outputs;
}

bool TcpIOProvider::isSystemReady() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return extendedProtocol_ ? (extendedState_.system_ready != 0)
                             : (legacyState_.system_ready != 0);
}

bool TcpIOProvider::isHardwareEmg() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return extendedProtocol_ ? (extendedState_.is_hardware_emg != 0)
                             : (legacyState_.is_hardware_emg != 0);
}

void TcpIOProvider::getCurrentPosition(double pos[6]) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (extendedProtocol_) {
        std::memcpy(pos, extendedState_.current_pos, sizeof(double) * 6);
    } else {
        std::memset(pos, 0, sizeof(double) * 6);
    }
}

void TcpIOProvider::getCurrentAxes(double axes[6]) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (extendedProtocol_) {
        std::memcpy(axes, extendedState_.current_axes, sizeof(double) * 6);
    } else {
        std::memset(axes, 0, sizeof(double) * 6);
    }
}

uint8_t TcpIOProvider::getCommandStatus() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return extendedProtocol_ ? extendedState_.cmd_status : 0;
}

// ─── Send commands ───

bool TcpIOProvider::sendLegacyCommand(uint8_t outputByte, bool softStop) {
    if (!connected_ || socket_fd_ < 0) return false;

    EcrtRobotCommand cmd{};
    cmd.cmd_id = cmdIdCounter_++;
    cmd.set_outputs = outputByte;
    cmd.soft_stops = softStop ? 1 : 0;

    int n = ::send(socket_fd_, &cmd, sizeof(cmd), 0);
    return n == sizeof(cmd);
}

bool TcpIOProvider::sendRobotCommand(uint8_t cmdType,
                                      const double coords[6],
                                      const double axes[6],
                                      double waitTime,
                                      uint8_t ioIndex, uint8_t ioValue) {
    if (!connected_ || socket_fd_ < 0) return false;

    if (!extendedProtocol_) {
        // Fallback: legacy protocol can only handle OUTPUT via set_outputs
        if (cmdType == GRS_CMD_OUTPUT) {
            uint8_t currentOutputs;
            {
                std::lock_guard<std::mutex> lock(stateMutex_);
                currentOutputs = legacyState_.outputs;
            }
            if (ioValue)
                currentOutputs |= (1u << ioIndex);
            else
                currentOutputs &= ~(1u << ioIndex);
            return sendLegacyCommand(currentOutputs);
        }
        // Non-output commands can't be sent via legacy protocol
        std::cerr << "[TCP] Warning: command type " << (int)cmdType
                  << " not supported in legacy protocol mode" << std::endl;
        return false;
    }

    GrsRobotCommand cmd{};
    cmd.cmd_id = cmdIdCounter_++;
    cmd.cmd_type = cmdType;
    cmd.io_index = ioIndex;
    cmd.io_value = ioValue;
    cmd.wait_time = waitTime;

    // Copy current output state for set_outputs
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        cmd.set_outputs = extendedState_.outputs;
    }

    if (coords) std::memcpy(cmd.coords, coords, sizeof(double) * 6);
    if (axes)   std::memcpy(cmd.axes, axes, sizeof(double) * 6);

    int n = ::send(socket_fd_, &cmd, sizeof(cmd), 0);
    return n == sizeof(cmd);
}

} // namespace grs_io
