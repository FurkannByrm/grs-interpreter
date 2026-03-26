#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <bitset>
#include <cstring>
#include "protocol.hpp"

std::atomic<bool> client_running{true};
int global_sock = 0;
static uint64_t cmd_id_counter = 1;

void command_sender_thread() {
    
    std::cout << "\n[CONTROL] Commands: '1' to LED ON, '0' to LED OFF, 'q' to quit\n";
    
    while (client_running) {
        char input;
        std::cin >> input; 

        GrsRobotCommand cmd{};
        cmd.cmd_id = cmd_id_counter++;
        cmd.soft_stops = 0;

        if (input == '1') {
            cmd.cmd_type = GRS_CMD_OUTPUT;
            cmd.io_index = 0;
            cmd.io_value = 1;
            send(global_sock, &cmd, sizeof(GrsRobotCommand), 0);
            std::cout << ">>> Command Sent: LED ON (ID:   " << cmd.cmd_id << ")\n";
        } 
        else if (input == '0') {
            cmd.cmd_type = GRS_CMD_OUTPUT;
            cmd.io_index = 0;
            cmd.io_value = 0;
            send(global_sock, &cmd, sizeof(GrsRobotCommand), 0);
            std::cout << ">>> Command Sent: LED OFF (ID:   " << cmd.cmd_id << ")\n";
        } 
        else if (input == 'q') {
            client_running = false;
        }
    }
}

int main(int argc, char const *argv[]) {
    const char* server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    struct sockaddr_in serv_addr;

    if ((global_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(12345);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    if (connect(global_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed!" << std::endl;
        return -1;
    }

    // Komut thread'ini başlat
    std::thread cmd_thread(command_sender_thread);

    GrsRobotState state;
    while (client_running) {
        int valread = recv(global_sock, &state, sizeof(GrsRobotState), MSG_WAITALL);
        if (valread <= 0) break;

        // Ekrana canlı I/O durumunu bas
        std::cout << "\r[MONITOR] IN: " << std::bitset<8>(state.inputs) 
                  << " | OUT: " << std::bitset<8>(state.outputs) 
                  << " | Last Seq: " << state.seq_id << "  " << std::flush;
    }

    client_running = false;
    if (cmd_thread.joinable()) cmd_thread.join();
    close(global_sock);
    return 0;
}
