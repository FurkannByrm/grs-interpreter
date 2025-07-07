#include "multiprocess/shared_mem.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main(){

    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    void* ptr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    SharedData* data = static_cast<SharedData*>(ptr);
    while (true) {
    if (data->gui_status == 1) {
        std::cout << "Received command: " << data->command << "\n";
        data->mc_status = 7;
    }
    usleep(1000); 
    }

    memset(data->status_packet, 0xAA, sizeof(data->status_packet));
    munmap(ptr, SHM_SIZE);
    close(fd); 


    return 0;
}