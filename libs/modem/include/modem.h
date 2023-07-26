#pragma once
#include <atomic>
#include <fcntl.h>
#include <lpp/cell_id.h>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <utility/optional.h>
#include <vector>

struct ATResult {
    char buffer[512];
    int  length;
    int  code;
};

enum ATCommand {
    AT         = 0,
    AT_CREG_2  = 1,
    AT_CREG_Q  = 2,
    AT_CIMI    = 3,
    AT_CIMI_Q  = 4,
    AT_CSURV_Q = 5,
    AT_CSQ     = 6,
    AT_QCRSRP  = 7,
    AT_QCRSRQ  = 8,
};

class Modem_AT {
public:
    explicit Modem_AT(const std::string& path, unsigned int baud_rate, CellID cell_id);
    virtual ~Modem_AT();

    bool                      initialize();
    Optional<CellID>          cell();
    Optional<double>          rsrq();
    std::vector<NeighborCell> neighbor_cells();

    virtual long long update_count() const { return cell_update_count; }
    virtual CellID    first_cell() const { return first_cell_id; }

protected:
    bool drain();
    bool read(ATResult* result);
    bool write(ATCommand command);

    bool update_signal_quality();
    bool update_neighbour_signal_quality();
    bool update_cell_id();

    void         query_routine();
    static void* query_routine_callback(void* userdata);

private:
    const std::string& path;
    unsigned int       baud_rate;

    std::atomic<long long> cell_update_count;
    CellID                 cell_id;
    CellID                 first_cell_id;

    int                       fd;
    termios                   newtio, oldtio;
    volatile bool             running;
    bool                      initialized;
    pthread_t                 thread;
    pthread_mutex_t           lock;
    int                       rssi;  // locked update
    std::vector<NeighborCell> neighbors;
};