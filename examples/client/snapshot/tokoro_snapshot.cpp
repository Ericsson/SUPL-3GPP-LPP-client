#ifdef ENABLE_TOKORO_SNAPSHOT

#include "tokoro_snapshot.hpp"

#include <loglet/loglet.hpp>
#include <msgpack/msgpack.hpp>

#include <fstream>

LOGLET_MODULE(tokoro_snapshot);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(tokoro_snapshot)

TokoroSnapshot::TokoroSnapshot(std::string const& output_dir, double sample_rate)
    : mOutputDir(output_dir), mSampleRate(sample_rate), mRng(std::random_device{}()) {
    INFOF("TokoroSnapshot initialized: dir=%s rate=%.2f%%", output_dir.c_str(),
          sample_rate * 100.0);
}

bool TokoroSnapshot::should_record() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(mRng) <= mSampleRate;
}

void TokoroSnapshot::record(generator::tokoro::SnapshotInput const& input) {
    auto filename = mOutputDir + "/tokoro_snapshot_" +
                    std::to_string(input.time.timestamp().seconds()) + ".msgpack";

    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        ERRORF("Failed to open file: %s", filename.c_str());
        return;
    }

    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);
    input.msgpack_pack(packer);

    ofs.write(reinterpret_cast<char const*>(buffer.data()), buffer.size());
    INFOF("Recorded snapshot: %s %zu bytes (%zu ephemeris, %zu orbit, %zu clock, %zu code, %zu "
          "phase)",
          filename.c_str(), buffer.size(), input.ephemeris.size(), input.orbit_corrections.size(),
          input.clock_corrections.size(), input.code_biases.size(), input.phase_biases.size());
}

#endif
