#include "modem.h"

#include <string.h>

static int rsrpToInt(double rsrp) {
    if (rsrp < -140) return 0;
    if (rsrp >= -44) return 97;
    return (int)(rsrp + 141);
}

static int rsrqToInt(double rsrq) {
    if (rsrq < -19.5) return 0;
    if (rsrq >= -3) return 34;
    return (int)(rsrq * 2 + 40);
}

static int decode_rssi(int r) {
    if (r == 0) return -113;
    if (r == 1) return -111;
    if (r >= 2 && r <= 30) return -109 + 2 * (r - 2);
    if (r == 31) return -51;
    return -999;
}

static constexpr const char* at_commands[] = {
    "AT",        "AT+CREG=2", "AT+CREG?",   "AT+CIMI",    "AT+CIMI?",
    "AT#CSURV?", "AT+CSQ",    "AT$QCRSRP?", "AT$QCRSRQ?",
};

Modem_AT::Modem_AT(const std::string& path, unsigned int baud_rate, CellID cell_id)
    : path(path), baud_rate(baud_rate), cell_update_count(0), cell_id(cell_id),
      first_cell_id(cell_id), fd(-1) {
    neighbors.reserve(16);
    initialized = false;
}

Modem_AT::~Modem_AT() {
    running = false;
    if (initialized) {
        pthread_join(thread, NULL);
        pthread_mutex_destroy(&lock);
    }

    if (fd >= 0) {
        tcsetattr(fd, TCSANOW, &oldtio);
        close(fd);
    }
}

bool Modem_AT::initialize() {
    // Modem using a direct connection (Modem will not have provide internet
    // access) Communicate with Modem using AT commands.
    fd = open(path.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        return false;
    }

    //
    // Code below is "heavily inspired" by the tutorial "Serial Programming HOWTO"
    // REFERENCE: https://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html

    /* save current serial port settings */
    tcgetattr(fd, &oldtio);
    /* clear struct for new port settings */
    memset(&newtio, 0, sizeof(newtio));

#define BAUDRATE B57600
    // B921600

    /*
      BAUDRATE: Set bps rate. You could also use cfsetispeed and
      cfsetospeed. CRTSCTS : output hardware flow control (only used if the
      cable has all necessary lines. See sect. 7 of Serial-HOWTO) CS8     :
      8n1 (8bit,no parity,1 stopbit) CLOCAL  : local connection, no modem
      contol CREAD   : enable receiving characters
    */
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

    /*
      IGNPAR  : ignore bytes with parity errors
      ICRNL   : map CR to NL (otherwise a CR input on the other computer
                will not terminate input)
      otherwise make device raw (no other input processing)
    */
    newtio.c_iflag = IGNPAR | ICRNL;

    /*
     Raw output.
    */
    newtio.c_oflag = 0;

    /*
      ICANON  : enable canonical input
      disable all echo functionality, and don't send signals to calling
      program
    */
    newtio.c_lflag = ICANON;

    /*
      initialize all control characters
      default values can be found in /usr/include/termios.h, and are given
      in the comments, but we don't need them here
    */
    newtio.c_cc[VINTR]    = 0; /* Ctrl-c */
    newtio.c_cc[VQUIT]    = 0; /* Ctrl-\ */
    newtio.c_cc[VERASE]   = 0; /* del */
    newtio.c_cc[VKILL]    = 0; /* @ */
    newtio.c_cc[VEOF]     = 4; /* Ctrl-d */
    newtio.c_cc[VTIME]    = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1; /* blocking read until 1 character arrives */
    newtio.c_cc[VSWTC]    = 0; /* '\0' */
    newtio.c_cc[VSTART]   = 0; /* Ctrl-q */
    newtio.c_cc[VSTOP]    = 0; /* Ctrl-s */
    newtio.c_cc[VSUSP]    = 0; /* Ctrl-z */
    newtio.c_cc[VEOL]     = 0; /* '\0' */
    newtio.c_cc[VREPRINT] = 0; /* Ctrl-r */
    newtio.c_cc[VDISCARD] = 0; /* Ctrl-u */
    newtio.c_cc[VWERASE]  = 0; /* Ctrl-w */
    newtio.c_cc[VLNEXT]   = 0; /* Ctrl-v */
    newtio.c_cc[VEOL2]    = 0; /* '\0' */

    /*
      now clean the modem line and activate the settings for the port
    */
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    running     = true;
    initialized = true;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&thread, NULL, query_routine_callback, this);
    return true;
}

Optional<CellID> Modem_AT::cell() {
    pthread_mutex_lock(&lock);
    auto id = cell_id;
    pthread_mutex_unlock(&lock);
    return id;
}

Optional<double> Modem_AT::rsrq() {
    pthread_mutex_lock(&lock);
    if (neighbors.size() == 0) {
        pthread_mutex_unlock(&lock);
        return {};
    }
    auto neighbor = neighbors[0];
    pthread_mutex_unlock(&lock);
    return neighbor.rsrp;
}

std::vector<NeighborCell> Modem_AT::neighbor_cells() {
    pthread_mutex_lock(&lock);
    auto copy = neighbors;
    pthread_mutex_unlock(&lock);
    return copy;
}

bool Modem_AT::drain() {
    if (fd < 0) return false;

    tcflush(fd, TCIOFLUSH);
    return true;
}

bool Modem_AT::read(ATResult* result) {
    if (fd < 0) return false;

    result->length = 0;
    for (;;) {
        char* buffer        = result->buffer + result->length;
        int   buffer_length = sizeof(result->buffer) - 1 - result->length;
        if (buffer_length <= 0) return false;

        // Wait a maximum of 5 seconds for a response
        struct timeval timeout;
        timeout.tv_sec  = 5;
        timeout.tv_usec = 0;

        fd_set set;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        auto rv = select(fd + 1, &set, NULL, NULL, &timeout);
        if (rv == -1)
            return false;
        else if (rv == 0)
            return false;

        int length = ::read(fd, buffer, buffer_length);

        // Skip periodic data from modem
        if (buffer[0] == '^') continue;
        if (buffer[0] == '\r') break;
        buffer[length] = 0;
        result->length += length;

        if (strcmp(buffer, "ERROR\n") == 0) break;
        if (strcmp(buffer, "OK\n") == 0) {
            result->code = 1;
            break;
        }
    }

    return true;
}

bool Modem_AT::write(ATCommand command) {
    if (fd < 0) return false;

    char buffer[256];
    sprintf(buffer, "%s\r", at_commands[command]);
    auto length = strlen(buffer);
    auto w      = ::write(fd, buffer, length);
    return w == (ssize_t)length;
}

bool Modem_AT::update_signal_quality() {
    // Request signal strength
    ATResult result{};
    if (!write(AT_CSQ)) return false;
    if (!read(&result)) return false;
    if (result.code != 1) return false;

    int rssi = 0;
    int ber  = 0;
    sscanf(result.buffer, " +CSQ: %i,%i", &rssi, &ber);

    auto rssi_value = decode_rssi(rssi);
    pthread_mutex_lock(&lock);
    this->rssi = rssi_value;
    pthread_mutex_unlock(&lock);
    return true;
}

bool Modem_AT::update_neighbour_signal_quality() {
    int          n_neighbors = 0;
    NeighborCell neighbors[16];

    // RSRP
    ATResult result{};
    if (!write(AT_QCRSRP)) return false;
    if (!read(&result)) return false;
    if (result.code != 1) return false;

    char* curLine = result.buffer;
    while (curLine) {
        int    id       = 0;
        int    EARFCN   = 0;
        double rsrp     = 0;
        char*  nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        if (strlen(curLine) > 10) {
            if (curLine[0] == '$') {
                memmove(curLine, curLine + 9, strlen(curLine) - 9 + 1);
            }
            char idString[30];
            strcpy(idString, curLine);
            const char s[2] = ",";
            strtok(idString, s);

            char EARFCNString[30];
            strcpy(EARFCNString, curLine + strlen(idString) + 1);
            strtok(EARFCNString, s);

            char rsrpString[30];
            strcpy(rsrpString, curLine + strlen(idString) + strlen(EARFCNString) + 3);
            strtok(rsrpString, s);
            rsrpString[strlen(rsrpString) - 2] = 0;

            id     = atoi(idString);
            EARFCN = atoi(EARFCNString);
            rsrp   = atof(rsrpString);

            neighbors[n_neighbors].id     = id;
            neighbors[n_neighbors].EARFCN = EARFCN;
            neighbors[n_neighbors++].rsrp = rsrpToInt(rsrp);
        }

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
    }

    // RSRQ
    if (!write(AT_QCRSRQ)) return false;
    if (!read(&result)) return false;
    if (result.code != 1) return false;

    curLine         = result.buffer;
    int n_neighbor2 = 0;
    while (curLine) {
        double rsrq     = 0;
        char*  nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        if (strlen(curLine) > 10) {
            if (curLine[0] == '$') {
                memmove(curLine, curLine + 9, strlen(curLine) - 9 + 1);
            }
            char rsrqString[256];
            strcpy(rsrqString, curLine);

            char* subString;  // the "result"

            subString = strtok(rsrqString, "\"");  // find the first double quote
            subString = strtok(NULL, "\"");        // find the second double quote

            if (subString) {
                rsrq                          = atof(subString);
                neighbors[n_neighbor2++].rsrq = rsrqToInt(rsrq);
            }

            for (int i = 1; i < n_neighbors; i++) {
                subString = strtok(NULL, "\"");  // find the next double quote
                subString = strtok(NULL, "\"");  // find the next double quote

                if (subString) {
                    rsrq                          = atof(subString);
                    neighbors[n_neighbor2++].rsrq = rsrqToInt(rsrq);
                }
            }
        }

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
    }
    pthread_mutex_lock(&lock);
    this->neighbors.clear();
    for (auto i = 0; i < n_neighbors; i++) {
        this->neighbors.push_back(neighbors[i]);
    }
    pthread_mutex_unlock(&lock);
    return true;
}

bool Modem_AT::update_cell_id() {
    ATResult result{};
    if (!write(AT)) return false;
    if (!read(&result)) return false;
    if (result.code != 1) return false;

    // Set mode 2 for CREG (LAC and CID information)
    if (!write(AT_CREG_2)) return false;
    if (!read(&result)) return false;
    if (result.code != 1) return false;

    if (!write(AT_CREG_Q)) return false;
    if (!read(&result)) return false;
    if (result.code != 1) return false;

    int          creg_n    = 0;
    int          creg_stat = 0;
    unsigned int lac       = 0;
    unsigned int cid       = 0;

    char temp[512];
    sscanf(result.buffer, "%*s %*s %s", temp);
    int count = sscanf(temp, "%i,%i,\"%x\",\"%x\"", &creg_n, &creg_stat, &lac, &cid);
    if (count != 4) return false;
    if (creg_n != 2)  // No LAC/CID reported
        return false;
    if (creg_stat != 1 && creg_stat != 5)  // No network registered
        return false;

    // Request MCC and MNC (really Internal Mobile Subscriber Identity where
    // MCC/MNC is in the begining)
    if (!write(AT_CIMI)) return false;
    if (!read(&result)) return false;
    if (result.code != 1)  // [OK]
        return false;

    int mcc = 0;
    int mnc = 5;
    sscanf(result.buffer, "%*s %s", temp);

    char mccbuff[4];
    memcpy(mccbuff, &temp[0], 3);
    mccbuff[3] = '\0';
    mcc        = atoi(mccbuff);

    char mncbuff[3];
    memcpy(mncbuff, &temp[3], 2);
    mncbuff[2] = '\0';
    mnc        = atoi(mncbuff);

    // char imsibuff[strlen(temp)];
    // memcpy( imsibuff, &temp[5], strlen(temp)-2-3);
    // imsibuff[strlen(imsibuff)-1] = '\0';

    pthread_mutex_lock(&lock);
    cell_id.mcc  = mcc;
    cell_id.mnc  = mnc;
    cell_id.tac  = lac;
    cell_id.cell = cid;
    cell_update_count++;
    pthread_mutex_unlock(&lock);
    return true;
}

void* Modem_AT::query_routine_callback(void* userdata) {
    auto modem = (Modem_AT*)userdata;
    modem->query_routine();
    return NULL;
}

void Modem_AT::query_routine() {
    while (running) {
        update_cell_id();
        update_neighbour_signal_quality();

        struct timespec ts;
        ts.tv_sec  = 2;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
    }
}
