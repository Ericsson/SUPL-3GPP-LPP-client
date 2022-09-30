#include <arpa/inet.h>
#include <fstream>
#include <getopt.h>
#include <lpp.h>
#include <modem.h>
#include <netinet/in.h>
#include <rtcm_generator.h>
#include <rtklib.h>
#include <sys/socket.h>
#include <unistd.h>

#define VERSION "v3.0.0 (public)"
#define SSR 0

static struct option long_options[] = {
    {"host", required_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {"mcc", required_argument, NULL, 'c'},
    {"mnc", required_argument, NULL, 'n'},
    {"cellid", required_argument, NULL, 'i'},
    {"tac", required_argument, NULL, 't'},
    {"msm_type", required_argument, NULL, 'y'},
    {"server_ip", required_argument, NULL, 'k'},
    {"server_port", required_argument, NULL, 'o'},
    {"file_output", required_argument, NULL, 'x'},
    {"modem_device", required_argument, NULL, 'm'},
    {"modem_baud_rate", required_argument, NULL, 'b'},
    {"output_device", required_argument, NULL, 'u'},
    {"output_baud_rate", required_argument, NULL, 'j'},
    {"ssl", no_argument, NULL, 's'},
    {"help", no_argument, NULL, '?'},
    {NULL, 0, NULL, 0},
};

struct Options {
    const char* host;
    int         port;
    bool        ssl;

    long mcc;
    long mnc;
    long tac;
    long cell_id;
    long msm_type;

    const char* server_ip;
    int         server_port;

    const char*  file_output;
    const char*  modem;
    unsigned int modem_baud_rate;
    const char*  output;
    unsigned int output_baud_rate;

    int help;
};

Options parse_arguments(int argc, char* argv[]) {
    Options options{};
    options.host             = nullptr;
    options.port             = 5431;
    options.ssl              = false;
    options.msm_type         = -1;
    options.server_ip        = nullptr;
    options.server_port      = 3000;
    options.mcc              = -1;
    options.mnc              = -1;
    options.tac              = -1;
    options.cell_id          = -1;
    options.modem            = nullptr;
    options.modem_baud_rate  = 9600;
    options.output           = nullptr;
    options.output_baud_rate = 115200;
    options.help             = 0;

    int c;
    int option_index;
    while ((c = getopt_long(argc, argv, "h:p:c:n:i:t:u:k:o:m:b:y:x:u:j:", long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'h': options.host = strdup(optarg); break;
        case 'p': options.port = atoi(optarg); break;
        case 's': options.ssl = true; break;
        case '?': options.help = true; break;

        case 'c': options.mcc = atoi(optarg); break;
        case 'n': options.mnc = atoi(optarg); break;
        case 'i': options.cell_id = atoi(optarg); break;
        case 't': options.tac = atoi(optarg); break;

        case 'y': options.msm_type = atoi(optarg); break;
        case 'k': options.server_ip = strdup(optarg); break;
        case 'o': options.server_port = atoi(optarg); break;

        case 'x': options.file_output = strdup(optarg); break;
        case 'm': options.modem = strdup(optarg); break;
        case 'b': options.modem_baud_rate = atoi(optarg); break;
        case 'u': options.output = strdup(optarg); break;
        case 'j': options.output_baud_rate = atoi(optarg); break;
        default: printf("ERROR: Unknown argument %c\n", c);
        }
    }

    return options;
}

void help(int argc, char* argv[]) {
    printf("options:\n");
    printf("  -?, --help              show help prompt\n");
    printf("  -h, --host              location server host "
           "[host]\n");
    printf("  -p, --port              location server post "
           "[integer]     default=5431\n");
    printf("  -s, --ssl               location server ssl  "
           "[flag]        default=off\n");
    printf("  -c, --mcc               mobile country code  "
           "[integer]\n");
    printf("  -n, --mnc               mobile network code  "
           "[integer]\n");
    printf("  -i, --cellid            cell id              "
           "[integer]\n");
    printf("  -t, --tac               tracking area code   "
           "[integer]\n");
    printf("  -y, --msm_type          msm type             "
           "[integer]     default=-1 (best suitable)\n");
    printf("  -k, --server_ip         server host          "
           "[host]        default=\"\" (no output server)\n");
    printf("  -o, --server_port       server port          "
           "[integer]     default=3000\n");
    printf("  -x, --file_output       file path output     "
           "[path]        default=\"\" (no output file)\n");
    printf("      --modem_device      modem path           "
           "[path]        default=\"\" (no modem connected)\n");
    printf("      --modem_baud_rate   modem baud rate      "
           "[integer]     default=9600\n");
    printf("\n");
}

int           connected = -1;
int           sockfd;
std::ofstream rtcm_file;
RTCMGenerator generator;
LPP_Client    client{false /* enable experimental segmentation support */};
CellID        cell;
Modem_AT*     modem;
stream_t      output_stream;
bool          output_stream_ok = false;

bool provide_location_information_callback(LocationInformation* location, void* userdata);
bool provide_ecid_callback(ECIDInformation* ecid, void* userdata);
void assistance_data_callback(LPP_Client*, LPP_Transaction* transaction, LPP_Message* message,
                              void* userdata);

int main(int argc, char* argv[]) {
    printf("SUPL-3GPP-LPP-client " VERSION "\n");

    auto options = parse_arguments(argc, argv);
    if (options.help) {
        help(argc, argv);
        return 0;
    }

    cell = CellID{
        .mcc  = options.mcc,
        .mnc  = options.mnc,
        .tac  = options.tac,
        .cell = options.cell_id,
    };

    printf("  location server: \"%s:%d\" %s\n", options.host ? options.host : "???", options.port,
           options.ssl ? "[ssl]" : "");
    printf("  cell identity:   %ld:%ld:%ld:%ld (mcc:mnc:tac:id)\n", cell.mcc, cell.mnc, cell.tac,
           cell.cell);

    if (options.host == nullptr) {
        printf("\n");
        help(argc, argv);
        fprintf(stderr, "ERROR: Location server host was not provided. Use --host\n");
        return 1;
    }

    if (options.mcc < 0 || options.mnc < 0 || options.tac < 0 || options.cell_id < 0) {
        printf("\n");
        help(argc, argv);
        fprintf(stderr,
                "ERROR: Cell identity was not provided. Use --mcc, --mnc, --tac, and --cellid\n");
        return 1;
    }

    if (options.file_output) {
        // Create output file
        printf("  output file:     \"%s\"\n", options.file_output);
        rtcm_file.open(options.file_output);

        if (!rtcm_file.is_open()) {
            fprintf(stderr, "ERROR: Unable to create or opening output file '%s'\n",
                    options.file_output);
            return 1;
        }
    }

    if (options.output) {
        // Create stream to device
        printf("  output device:   \"%s\" (baudrate %u)\n", options.output,
               options.output_baud_rate);

        strinit(&output_stream);

        char path[1024];
        sprintf(path, "%s:%i:8:n:1:off", options.output, options.output_baud_rate);

        int result = stropen(&output_stream, STR_SERIAL, STR_MODE_RW, path);
        if (result != 1) {
            fprintf(stderr,
                    "ERROR: Unable to open stream to device '%s' (/dev/ should not be included)\n",
                    path);
            return 1;
        }

        output_stream_ok = true;
    }

    if (options.server_ip) {
        // Connect to output server
        printf("  output server:   \"%s:%d\"\n", options.server_ip, options.server_port);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            fprintf(stderr, "ERROR: Unable to create socket for output server \"%s:%d\"\n",
                    options.server_ip, options.server_port);
            return 1;
        }

        struct sockaddr_in server_addr;
        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family      = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(options.server_ip);
        server_addr.sin_port        = htons(options.server_port);

        connected = connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));
        if (connected != 0) {
            fprintf(stderr, "ERROR: Unable to connect to output server\n");
            return 1;
        }
    }

    // Initialize OpenSSL
    network_initialize();

    // Register callback for when the location server request location
    // information from the device.
    client.provide_location_information_callback(NULL, provide_location_information_callback);

    // Register callback for when the location server request ECID and
    // measurements from the device.
    client.provide_ecid_callback(NULL, provide_ecid_callback);

    // Connect to location server
    if (!client.connect(options.host, options.port, options.ssl, cell)) {
        fprintf(stderr, "ERROR: Unable to connect to location server \"%s:%i\"%s\n", options.host,
                options.port, options.ssl ? " [ssl]" : "");
        return 0;
    }

    // Initialize Modem
    if (options.modem) {
        modem = new Modem_AT(options.modem, options.modem_baud_rate, cell);
        if (!modem->initialize()) {
            fprintf(stderr, "ERROR: Unable to connect to modem \"%s\" (baudrate: %u)\n",
                    options.modem, options.modem_baud_rate);
            return 0;
        }
    }

    // Enable generation of message for GPS, GLONASS, Galileo, and Beidou.
    auto gnss = GNSSSystems{
        .gps     = true,
        .glonass = true,
        .galileo = true,
        .beidou  = true,
    };

    printf("  gnss support:   ");
    if (gnss.gps) printf(" GPS");
    if (gnss.glonass) printf(" GLO");
    if (gnss.galileo) printf(" GAL");
    if (gnss.beidou) printf(" BDS");
    printf("\n");

    // Filter what messages you want generated. (MSM6 is not supported)
    auto filter = MessageFilter{};
    if (options.msm_type == 4) {
        filter.msm.msm4 = true;
        filter.msm.msm5 = false;
        filter.msm.msm7 = false;
    } else if (options.msm_type == 5) {
        filter.msm.msm4 = false;
        filter.msm.msm5 = true;
        filter.msm.msm7 = false;
    } else if (options.msm_type == 7) {
        filter.msm.msm4 = false;
        filter.msm.msm5 = false;
        filter.msm.msm7 = true;
    }

    printf("  msm support:    ");
    if (filter.msm.msm4) printf(" MSM4");
    if (filter.msm.msm5) printf(" MSM5");
    if (filter.msm.msm7) printf(" MSM7");
    printf("\n");

    // Create RTCM generator for converting LPP messages to RTCM messages.
    generator = RTCMGenerator{gnss, filter};

#if !SSR
    // Request assistance data (OSR) from server for the 'cell' and register a callback
    // that will be called when we receive assistance data.
    auto request = client.request_assistance_data(cell, NULL, assistance_data_callback);
#else
    // Request assistance data (SSR) from server for the 'cell' and register a callback
    // that will be called when we receive assistance data.
    auto request = client.request_assistance_data_ssr(cell, NULL, assistance_data_callback);
#endif

    if (request == AD_REQUEST_INVALID) {
        // Request for assistance data failed
        fprintf(stderr,
                "ERROR: Requesting assistance data failed (no response from location server)\n");
        return 0;
    }

    // Run LPP client
    for (;;) {
        struct timespec timeout;
        timeout.tv_sec  = 0;
        timeout.tv_nsec = 1000000 * 100;  // 100 ms
        nanosleep(&timeout, NULL);

        // client.process() MUST be called at least once every second, otherwise
        // ProvideLocationInformation messages will not be send to the server.
        if (!client.process()) {
            fprintf(stderr, "ERROR: Client has lost connection to location server\n");
            return 0;
        }
    }

    network_cleanup();
    return EXIT_SUCCESS;
}

bool provide_location_information_callback(LocationInformation* location, UNUSED void* userdata) {
    location->time = time(NULL);
    location->lat  = 20;
    location->lon  = 20;
    return true;
}

bool provide_ecid_callback(ECIDInformation* ecid, UNUSED void* userdata) {
    if (!modem) return false;

    auto neighbors = modem->neighbor_cells();
    auto cell      = modem->cell();
    if (!cell.initialized()) return false;

    ecid->cell           = cell.value();
    ecid->neighbor_count = 0;

    for (auto& neighbor_cell : neighbors) {
        if (ecid->neighbor_count < 16) {
            ecid->neighbors[ecid->neighbor_count++] = {
                .id     = neighbor_cell.id,
                .earfcn = neighbor_cell.EARFCN,
                .rsrp   = neighbor_cell.rsrp,
                .rsrq   = neighbor_cell.rsrq,
            };
        }
    }

    return true;
}

void assistance_data_callback(LPP_Client*, LPP_Transaction*, LPP_Message* message, void*) {
#if !SSR
    if (!generator.process(message)) {
        // Segmented LPP message, waiting for rest before converting to RTCM.
        return;
    }

    // Convert LPP messages to buffer of RTCM messages.
    Generated     generated_messages{};
    unsigned char buffer[4 * 4096];
    auto          size   = sizeof(buffer);
    auto          length = generator.convert(buffer, &size, &generated_messages);

    printf("length: %4zu | msm%i | ", length, generated_messages.msm);
    if (generated_messages.mt1074) printf("1074 ");
    if (generated_messages.mt1075) printf("1075 ");
    if (generated_messages.mt1076) printf("1076 ");
    if (generated_messages.mt1077) printf("1077 ");
    if (generated_messages.mt1084) printf("1084 ");
    if (generated_messages.mt1085) printf("1085 ");
    if (generated_messages.mt1086) printf("1086 ");
    if (generated_messages.mt1087) printf("1087 ");
    if (generated_messages.mt1094) printf("1094 ");
    if (generated_messages.mt1095) printf("1095 ");
    if (generated_messages.mt1096) printf("1096 ");
    if (generated_messages.mt1097) printf("1097 ");
    if (generated_messages.mt1124) printf("1124 ");
    if (generated_messages.mt1125) printf("1125 ");
    if (generated_messages.mt1126) printf("1126 ");
    if (generated_messages.mt1127) printf("1127 ");
    if (generated_messages.mt1030) printf("1030 ");
    if (generated_messages.mt1031) printf("1031 ");
    if (generated_messages.mt1230) printf("1230 ");
    if (generated_messages.mt1006) printf("1006 ");
    if (generated_messages.mt1032) printf("1032 ");
    printf("\n");
    if (length > 0) {
        if (rtcm_file.is_open()) {
            rtcm_file.write((char*)buffer, length);
            rtcm_file.flush();
        }

        if (output_stream_ok) {
            strwrite(&output_stream, buffer, length);
        }

        if (connected == 0) {
            auto result = write(sockfd, buffer, length);
            if (result == -1) {
                connected = 0;
            }
        }
    }
#else
    // Extract SSR data
    printf("SSR LPP Message: %p\n", (void*)message);
#endif
}
