#pragma once
#include <cstdint>

#define SHM_NAME "/my_motion_shared_mem"
#define SHM_SIZE sizeof(SharedData)

struct SharedData {
    uint8_t gui_status;
    uint8_t mc_status;
    uint32_t program_info;
    char command[256];
    uint8_t status_packet[160];
};
