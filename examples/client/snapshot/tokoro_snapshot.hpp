#pragma once

#ifdef ENABLE_TOKORO_SNAPSHOT

#include <generator/tokoro/snapshot.hpp>

#include <random>
#include <string>

class TokoroSnapshot {
public:
    TokoroSnapshot(std::string const& output_dir, double sample_rate);

    bool should_record();
    void record(generator::tokoro::SnapshotInput const& input);

private:
    std::string  mOutputDir;
    double       mSampleRate;
    std::mt19937 mRng;
};

#endif
