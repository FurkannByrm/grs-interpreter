#include "io/tcp_io_provider.hpp"
#include <iostream>
#include <cstring>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <poll.h>
#include <cerrno>

namespace grs_io {

TcpIOProvider::TcpIOProvider(const std::string& host, int port)
    : host_(host), port_(port) {
    std::memset(&state_, 0, sizeof(state_));
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
        struct pollfd pfd;
        pfd.fd = socket_fd_;
        pfd.events = POLLOUT;
        int pollRet = poll(&pfd, 1, 2000);

        if (pollRet <= 0) {
            std::cerr << "[TCP] Connection timeout to " << host_ << ":" << port_ << std::endl;
            ::close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

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

    // Restore blocking mode
    fcntl(socket_fd_, F_SETFL, flags);

    connected_ = true;
    running_ = true;

    recvThread_ = std::thread(&TcpIOProvider::recvLoop, this);

    std::cerr << "[TCP] Connected to " << host_ << ":" << port_
              << " (unified 128-byte protocol)" << std::endl;
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
    GrsRobotState incoming;
    while (running_ && connected_) {
        int n = ::recv(socket_fd_, &incoming, sizeof(incoming), MSG_WAITALL);
        if (n <= 0) {
            connected_ = false;
            running_ = false;
            break;
        }
        if (n == sizeof(incoming)) {
            std::lock_guard<std::mutex> lock(stateMutex_);
            state_ = incoming;
        }
    }
}

// ─── IOProvider interface ───

bool TcpIOProvider::readDigitalInput(uint8_t index) {
    if (index >= 8) return false;
    std::lock_guard<std::mutex> lock(stateMutex_);
    return (state_.inputs >> index) & 1;
}

void TcpIOProvider::writeDigitalOutput(uint8_t index, bool value) {
    if (index >= 8 || !connected_) return;

    double zeroCoords[6] = {};
    double zeroAxes[6] = {};
    sendRobotCommand(GRS_CMD_OUTPUT, zeroCoords, zeroAxes, 0.0, index, value ? 1 : 0);
}

bool TcpIOProvider::readDigitalOutput(uint8_t index) {
    if (index >= 8) return false;
    std::lock_guard<std::mutex> lock(stateMutex_);
    return (state_.outputs >> index) & 1;
}

// ─── State info ───

uint8_t TcpIOProvider::getInputByte() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return state_.inputs;
}

uint8_t TcpIOProvider::getOutputByte() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return state_.outputs;
}

bool TcpIOProvider::isSystemReady() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return state_.system_ready != 0;
}

bool TcpIOProvider::isHardwareEmg() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return state_.is_hardware_emg != 0;
}

void TcpIOProvider::getCurrentPosition(double pos[6]) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    std::memcpy(pos, state_.current_pos, sizeof(double) * 6);
}

void TcpIOProvider::getCurrentAxes(double axes[6]) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    std::memcpy(axes, state_.current_axes, sizeof(double) * 6);
}

uint8_t TcpIOProvider::getCommandStatus() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return state_.cmd_status;
}

// ─── Send commands ───

bool TcpIOProvider::sendRobotCommand(uint8_t cmdType,
                                      const double coords[6],
                                      const double axes[6],
                                      double waitTime,
                                      uint8_t ioIndex, uint8_t ioValue) {
    if (!connected_ || socket_fd_ < 0) return false;

    GrsRobotCommand cmd{};
    cmd.cmd_id = cmdIdCounter_++;
    cmd.cmd_type = cmdType;
    cmd.io_index = ioIndex;
    cmd.io_value = ioValue;
    cmd.wait_time = waitTime;

    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        cmd.set_outputs = state_.outputs;
    }

    if (coords) std::memcpy(cmd.coords, coords, sizeof(double) * 6);
    if (axes)   std::memcpy(cmd.axes, axes, sizeof(double) * 6);

    int n = ::send(socket_fd_, &cmd, sizeof(cmd), 0);
    return n == sizeof(cmd);
}

} // namespace grs_io
