#include <iostream>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <ecrt.h>
#include <math.h>
#include <sched.h>
#include <string.h>

// --- Application Parameters ---
#define TASK_FREQUENCY   1000          // Hz
#define TASK_PERIOD_NS   1000000ULL    // 1 ms in nanoseconds
#define DC_SYNC0_SHIFT   100000ULL     // 100 µs output shift

// --- Hardware IDs ---
#define Beckhoff_VID 0x00000002
#define EL7211_PID   0x1c2b3052        // EL7211-0010 product code

// Slave configurations
#define EL7211_POS 0, 1 // Alias 0, Position 1

// --- EtherCAT Master Objects ---
static ec_master_t *master = NULL;
static ec_domain_t *domain1 = NULL;
static uint8_t *domain1_pd = NULL; // Process data pointer
static ec_slave_config_t *sc_el7211 = NULL;

// --- PDO Offsets in Domain Memory ---
static unsigned int off_el7211_ctrl;
static unsigned int off_el7211_target_vel;
static unsigned int off_el7211_position;
static unsigned int off_el7211_status;
static unsigned int off_el7211_vel_actual;

// --- Domain Registration Array ---
// EL7211 uses custom object indices:
// RxPDO 0x1600: 0x7010:01 (Controlword, 16-bit)
// RxPDO 0x1601: 0x7010:06 (Target velocity, 32-bit)
// TxPDO 0x1a00: 0x6000:11 (Position, 32-bit)
// TxPDO 0x1a01: 0x6010:01 (Statusword, 16-bit)
// TxPDO 0x1a02: 0x6010:07 (Velocity actual, 32-bit)
const static ec_pdo_entry_reg_t domain1_regs[] = {
    {EL7211_POS, Beckhoff_VID, EL7211_PID, 0x7010, 0x01, &off_el7211_ctrl},        // Controlword
    {EL7211_POS, Beckhoff_VID, EL7211_PID, 0x7010, 0x06, &off_el7211_target_vel},  // Target Velocity
    {EL7211_POS, Beckhoff_VID, EL7211_PID, 0x6000, 0x11, &off_el7211_position},    // Position
    {EL7211_POS, Beckhoff_VID, EL7211_PID, 0x6010, 0x01, &off_el7211_status},      // Statusword
    {EL7211_POS, Beckhoff_VID, EL7211_PID, 0x6010, 0x07, &off_el7211_vel_actual},  // Velocity actual
    {}
};

// --- PDO Mappings for EL7211-0010 ---
// The EL7211 has a fixed PDO structure defined in the device's SII:
// SM2 (RxPDO, output to slave):
//   0x1600 -> 0x7010:01 (Controlword)
//   0x1601 -> 0x7010:06 (Target velocity)
// SM3 (TxPDO, input from slave):
//   0x1a00 -> 0x6000:11 (Position)
//   0x1a01 -> 0x6010:01 (Statusword)
//   0x1a02 -> 0x6010:07 (Velocity actual)
static ec_pdo_entry_info_t el7211_pdo_entries[] = {
    {0x7010, 0x01, 16}, // Controlword
    {0x7010, 0x06, 32}, // Target Velocity
    {0x6000, 0x11, 32}, // Position
    {0x6010, 0x01, 16}, // Statusword
    {0x6010, 0x07, 32}, // Velocity actual value
};
static ec_pdo_info_t el7211_pdos[] = {
    {0x1600, 1, el7211_pdo_entries + 0}, // SM2 RxPDO: Controlword
    {0x1601, 1, el7211_pdo_entries + 1}, // SM2 RxPDO: Target velocity
    {0x1a00, 1, el7211_pdo_entries + 2}, // SM3 TxPDO: Position
    {0x1a01, 1, el7211_pdo_entries + 3}, // SM3 TxPDO: Statusword
    {0x1a02, 1, el7211_pdo_entries + 4}, // SM3 TxPDO: Velocity actual
};
static ec_sync_info_t el7211_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT,  0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 2, el7211_pdos + 0, EC_WD_DISABLE},
    {3, EC_DIR_INPUT,  3, el7211_pdos + 2, EC_WD_DISABLE},
    {0xff}
};
// ------------------------------------------------------------------

void cyclic_task() {
    struct timespec wakeup_time;
    clock_gettime(CLOCK_MONOTONIC, &wakeup_time);

    uint16_t status_word = 0;
    uint16_t control_word = 0;
    int enable_step = 0;

    // Velocity ramp state — reset on every entry to Operation Enabled
    int fault_count = 0;  // counts drive fault events for diagnostics
    double   vel_t       = 0.0;   // local time, only counts while enabled
    int32_t  vel_cmd     = 0;     // ramped command sent to drive
    bool     was_enabled = false; // tracks re-entry to reset state

    while (1) {
        // 1. Calculate next wakeup time (1ms)
        wakeup_time.tv_nsec += 1000000;
        while (wakeup_time.tv_nsec >= 1000000000) {
            wakeup_time.tv_nsec -= 1000000000;
            wakeup_time.tv_sec++;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);

        // 2. Receive process data
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);

        // 3. Read Inputs from EL7211
        status_word = EC_READ_U16(domain1_pd + off_el7211_status);
        int32_t current_pos = EC_READ_S32(domain1_pd + off_el7211_position);
        int32_t current_vel = EC_READ_S32(domain1_pd + off_el7211_vel_actual);

        // 4. DS402 State Machine (EL7211 uses DS402 via 0x7010:01/0x6010:01)
        // SW=0x60 = "Switch On Disabled" (bits 5+6 set)
        // SW=0x21 = "Ready to Switch On"
        // SW=0x23 = "Switched On"
        // SW=0x27 = "Operation Enabled" → send velocity here
        uint16_t ds402_state = status_word & 0x006F;

        if (status_word & 0x0008) {
            // Fault (bit 3) - reset sequence
            if (was_enabled) { fault_count++; }  // count transitions into fault
            was_enabled = false;   // will reset vel_cmd when re-enabled
            vel_cmd = 0;
            EC_WRITE_S32(domain1_pd + off_el7211_target_vel, 0);
            if (++enable_step < 8) {
                control_word = 0x0000;
            } else if (enable_step == 8) {
                control_word = 0x0080; // Fault reset pulse
            } else {
                control_word = 0x0000;
                if (enable_step > 16) enable_step = 0;
            }
        } else {
            enable_step = 0;
            switch (ds402_state) {
                case 0x0060: // Switch On Disabled
                case 0x0040: // Switch On Disabled (alt)
                    was_enabled = false;
                    vel_cmd = 0;
                    EC_WRITE_S32(domain1_pd + off_el7211_target_vel, 0);
                    control_word = 0x0006; // Shutdown → go to Ready to Switch On
                    break;
                case 0x0021: // Ready to Switch On
                    control_word = 0x0007; // Switch On
                    break;
                case 0x0023: // Switched On
                    control_word = 0x000F; // Enable Operation
                    break;
                case 0x0027: { // Operation Enabled - SEND VELOCITY
                    control_word = 0x000F;

                    // Reset ramp state on every fresh entry into this state
                    if (!was_enabled) {
                        vel_t       = 0.0;
                        vel_cmd     = 0;
                        was_enabled = true;
                        EC_WRITE_S32(domain1_pd + off_el7211_target_vel, 0);
                        break;  // one safe cycle at zero before ramping
                    }

                    vel_t += 0.001; // advance local timer (seconds)

                    // Observed scale: actual_RPM ≈ 2 × (vcmd/174763 × 10)
                    // To stay safe and find fault threshold: use very small amplitude.
                    // 8000 inc/s ≈ 0.46 RPM commanded → ~1 RPM actual (conservative)
                    // Ramp: 80 inc/s per ms → reaches 8000 in 100 ms
                    const double  VEL_AMP   = 8000.0;   // very slow
                    const int32_t MAX_ACCEL = 80;        // per 1ms cycle

                    // Very slow sine: 60 second period (0.0167 Hz)
                    int32_t vel_target = (int32_t)(VEL_AMP * sin(2.0 * M_PI * (1.0/60.0) * vel_t));

                    int32_t delta = vel_target - vel_cmd;
                    if (delta >  MAX_ACCEL) delta =  MAX_ACCEL;
                    if (delta < -MAX_ACCEL) delta = -MAX_ACCEL;
                    vel_cmd += delta;

                    EC_WRITE_S32(domain1_pd + off_el7211_target_vel, vel_cmd);
                    break;
                }
                default:
                    was_enabled = false;
                    vel_cmd = 0;
                    EC_WRITE_S32(domain1_pd + off_el7211_target_vel, 0);
                    control_word = 0x0006;
                    break;
            }
        }

        static int dbg_counter = 0;
        if (++dbg_counter >= 500) {
            dbg_counter = 0;

            ec_master_state_t ms;
            ec_domain_state_t ds;
            ecrt_master_state(master, &ms);
            ecrt_domain_state(domain1, &ds);

            std::cout << "[MASTER] slaves=" << ms.slaves_responding
                      << " al=0x" << std::hex << (int)ms.al_states
                      << std::dec << " link=" << ms.link_up << std::endl;
            std::cout << "[DOMAIN] wc=" << ds.working_counter
                      << " state=" << (int)ds.wc_state << std::endl;
            std::cout << "[STATUS] SW=0x" << std::hex << status_word
                      << " state=0x" << ds402_state
                      << " CW=0x" << control_word
                      << std::dec
                      << " pos=" << current_pos
                      << " vcmd=" << vel_cmd
                      << " vel=" << current_vel
                      << " faults=" << fault_count << std::endl;
        }

        // 5. Write Outputs to EL7211
        EC_WRITE_U16(domain1_pd + off_el7211_ctrl, control_word);

        // 6. Provide application time from CLOCK_MONOTONIC and synchronize DC clocks
        struct timespec app_ts;
        clock_gettime(CLOCK_MONOTONIC, &app_ts);
        uint64_t app_time = (uint64_t)app_ts.tv_sec * 1000000000ULL + (uint64_t)app_ts.tv_nsec;
        ecrt_master_application_time(master, app_time);
        ecrt_master_sync_reference_clock(master);
        ecrt_master_sync_slave_clocks(master);

        // 7. Send Process Data
        ecrt_domain_queue(domain1);
        ecrt_master_send(master);
    }
}

int main(int argc, char **argv) {
    // Prevent memory swapping for real-time performance
    mlockall(MCL_CURRENT | MCL_FUTURE);

    std::cout << "Requesting EtherCAT Master..." << std::endl;
    master = ecrt_request_master(0);
    if (!master) return -1;

    domain1 = ecrt_master_create_domain(master);
    if (!domain1) return -1;

    std::cout << "Configuring EL7211 Slave..." << std::endl;
    sc_el7211 = ecrt_master_slave_config(master, EL7211_POS, Beckhoff_VID, EL7211_PID);
    if (!sc_el7211) return -1;

    // Apply PDO configuration for EL7211
    // EL7211 has fixed PDO structure: 2 RxPDOs (0x1600, 0x1601) + 3 TxPDOs (0x1a00, 0x1a01, 0x1a02)
    if (ecrt_slave_config_pdos(sc_el7211, EC_END, el7211_syncs)) {
        std::cerr << "Failed to configure PDOs." << std::endl;
        return -1;
    }
    std::cout << "PDO configuration applied." << std::endl;

    // DC Synchronization 
    // EL7211 requires DC sync but with different configuration than EL7221
    // Use assign_activate=0x0700 (SYNC0 active, matched with cycle time)
    ecrt_slave_config_dc(sc_el7211, 0x0700, TASK_PERIOD_NS, DC_SYNC0_SHIFT, 0, 0);
    std::cout << "DC sync configured (0x0700)." << std::endl;

    // Set Mode of Operation to CSV (Cyclic Synchronous Velocity = 9)
    // EL7211 uses 0x7010:03 instead of the standard DS402 0x6060
    ecrt_slave_config_sdo8(sc_el7211, 0x7010, 0x03, 9);
    std::cout << "Mode of Operation set to CSV (9)." << std::endl;

    // Register PDO entries to the domain
    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs)) {
        std::cerr << "PDO entry registration failed." << std::endl;
        return -1;
    }

    std::cout << "Activating Master..." << std::endl;
    if (ecrt_master_activate(master)) {
        std::cerr << "Master activation failed." << std::endl;
        return -1;
    }

    domain1_pd = ecrt_domain_data(domain1);

    // --- LINUX GERÇEK ZAMANLI (REAL-TIME) ÖNCELİK AYARI ---
    std::cout << "Setting Real-Time Priority (SCHED_FIFO)..." << std::endl;
    struct sched_param param = {};
    param.sched_priority = 98; 
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        std::cerr << "UYARI: Gercek zamanli oncelik atanamadi! (sudo ile calistirdigindan emin ol)" << std::endl;
    }
    // ------------------------------------------------------

    std::cout << "Starting Cyclic Loop..." << std::endl;
    cyclic_task(); 

    return 0;
}
