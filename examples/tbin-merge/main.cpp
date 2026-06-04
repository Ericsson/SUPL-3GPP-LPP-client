#include <format/tbin/reader.hpp>
#include <format/tbin/writer.hpp>

#include <cstdio>
#include <queue>
#include <string>
#include <vector>

struct Entry {
    int64_t  timestamp_us;
    uint32_t reader_index;
    bool     operator>(Entry const& o) const { return timestamp_us > o.timestamp_us; }
};

int main(int argc, char* argv[]) {
    std::string              output_path;
    std::vector<std::string> inputs;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            fprintf(stderr, "Usage: tbin-merge [-o output.tbin] input1.tbin input2.tbin ...\n");
            return 0;
        } else {
            inputs.push_back(arg);
        }
    }

    if (inputs.empty()) {
        fprintf(stderr, "error: no input files\n");
        return 1;
    }

    std::vector<format::tbin::Reader>                                   readers(inputs.size());
    std::vector<format::tbin::Message>                                  pending(inputs.size());
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;

    for (size_t i = 0; i < inputs.size(); i++) {
        if (!readers[i].open(inputs[i])) {
            fprintf(stderr, "error: cannot open %s\n", inputs[i].c_str());
            return 1;
        }
        if (readers[i].next(pending[i])) {
            pq.push({pending[i].timestamp_us, static_cast<uint32_t>(i)});
        }
    }

    format::tbin::Writer writer;
    if (!output_path.empty()) {
        if (!writer.open(output_path, "merged")) {
            fprintf(stderr, "error: cannot open output %s\n", output_path.c_str());
            return 1;
        }
    } else {
        if (!writer.open("/dev/stdout", "merged")) {
            fprintf(stderr, "error: cannot write to stdout\n");
            return 1;
        }
    }

    uint64_t count = 0;
    while (!pq.empty()) {
        auto top = pq.top();
        pq.pop();

        auto& msg = pending[top.reader_index];
        writer.write(msg.timestamp_us, msg.data.data(), static_cast<uint32_t>(msg.data.size()));
        count++;

        if (readers[top.reader_index].next(pending[top.reader_index])) {
            pq.push({pending[top.reader_index].timestamp_us, top.reader_index});
        }
    }

    writer.close();
    fprintf(stderr, "merged %lu messages from %zu files\n", count, inputs.size());
    return 0;
}
