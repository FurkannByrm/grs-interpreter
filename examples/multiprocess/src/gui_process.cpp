#include "multiprocess/shared_mem.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <iostream>



int main(){

int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
ftruncate(fd,SHM_SIZE);
void* ptr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
SharedData* data = static_cast<SharedData*>(ptr);
data->gui_status = 1;
data->program_info = 42;

strcpy(data->command, "MOVE X9 Y0 Z0");
std::cout<<"GUI is writed: command = "<<data->command<<"\n";

munmap(ptr, SHM_SIZE);
close(fd);
return 0;

}   
