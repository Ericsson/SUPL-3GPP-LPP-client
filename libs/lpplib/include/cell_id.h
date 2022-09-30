#pragma once

struct CellID {
    long mcc;
    long mnc;
    long tac;
    long cell;
};

struct NeighborCell {
    int id;
    int EARFCN;
    int rsrp;
    int rsrq;
};

inline bool operator==(CellID a, CellID b) {
    return a.mcc == b.mcc && a.mnc == b.mnc && a.tac == b.tac &&
           a.cell == b.cell;
}

inline bool operator!=(CellID a, CellID b) {
    return !(a == b);
}
