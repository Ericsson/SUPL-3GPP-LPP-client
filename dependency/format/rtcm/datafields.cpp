#include "datafields.hpp"
#include <iostream>

LOGLET_MODULE3(format, rtcm, datafield);

namespace std {
ostream& operator<<(ostream& os, const DF009 d) {
    return os << "      satellite id:     " << static_cast<int>(d) << "\n";
}
ostream& operator<<(ostream& os, const DF071 d) {
    return os << "      GPS IODE:         " << static_cast<int>(d) << "\n";
}
ostream& operator<<(ostream& os, const DF076 d) {
    return os << "      GPS week number:  " << static_cast<int>(d) << "\n";
}
ostream& operator<<(ostream& os, const DF077 d) {
    return os << "      SV accuracy:      " << static_cast<int>(d) << " meters\n";
}
ostream& operator<<(ostream& os, const DF078 d) {
    return os << "      GPS code on L2:   " << (d ? "true" : "false") << "\n";
}
ostream& operator<<(ostream& os, const DF079 d) {
    return os << "      GPS IDOT:         " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF081 d) {
    return os << "      t_oc:             " << static_cast<double>(d) << " seconds\n";
}
ostream& operator<<(ostream& os, const DF082 d) {
    return os << "      a_f2:             " << static_cast<double>(d) << " s/s^2\n";
}
ostream& operator<<(ostream& os, const DF083 d) {
    return os << "      a_f1:             " << static_cast<double>(d) << " s/s\n";
}
ostream& operator<<(ostream& os, const DF084 d) {
    return os << "      a_f0:             " << static_cast<double>(d) << " s\n";
}
ostream& operator<<(ostream& os, const DF085 d) {
    return os << "      IODC:             " << static_cast<int>(d) << " s\n";
}
ostream& operator<<(ostream& os, const DF086 d) {
    return os << "      C_rs:             " << static_cast<double>(d) << " meters\n";
}
ostream& operator<<(ostream& os, const DF087 d) {
    return os << "      delta n:          " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF088 d) {
    return os << "      M_0:              " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF089 d) {
    return os << "      C_uc              " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF090 d) {
    return os << "      GPS eccentricity: " << static_cast<double>(d) << " \n";
}
ostream& operator<<(ostream& os, const DF091 d) {
    return os << "      C_us:             " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF092 d) {
    return os << "      sqrt(A):          " << static_cast<double>(d) << " m^0.5\n";
}
ostream& operator<<(ostream& os, const DF093 d) {
    return os << "      t_oe:             " << static_cast<double>(d) << " seconds\n";
}
ostream& operator<<(ostream& os, const DF094 d) {
    return os << "      C_ic:             " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF095 d) {
    return os << "      OMEGA 0:          " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF096 d) {
    return os << "      C_is:             " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF097 d) {
    return os << "      i_0               " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF098 d) {
    return os << "      C_rc:             " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF099 d) {
    return os << "      GPS omega:        " << static_cast<double>(d) << " radians\n";
}
ostream& operator<<(ostream& os, const DF100 d) {
    return os << "      GPS omega dot:    " << static_cast<double>(d) << " radians/s\n";
}
ostream& operator<<(ostream& os, const DF101 d) {
    return os << "      GPS t_GD:         " << static_cast<double>(d) << " seconds\n";
}
ostream& operator<<(ostream& os, const DF102 d) {
    return os << "      GPS SV HEALTH:    " << static_cast<int>(d) << " \n";
}
ostream& operator<<(ostream& os, const DF103 d) {
    return os << "      GPS L2 P flag:    " << (d ? "OFF" : "ON") << " \n";
}
ostream& operator<<(ostream& os, const DF137 d) {
    return os << "      GPS Fit Interval: " << (d ? ">4 hours" : "=4 hours") << " \n";
}
ostream& operator<<(ostream& os, const DF252 d) {
    return os << "      Galileo PRN:      " << static_cast<int>(d) << " \n";
}
ostream& operator<<(ostream& os, const DF286 d) {
    return os << "      SISA index:       " << static_cast<int>(d) << " \n";
}
ostream& operator<<(ostream& os, const DF289 d) {
    return os << "      Galileo week:     " << static_cast<int>(d) << " \n";
}
ostream& operator<<(ostream& os, const DF290 d) {
    return os << "      Galileo IODnav:   " << static_cast<int>(d) << " \n";
}
ostream& operator<<(ostream& os, const DF292 d) {
    return os << "      Galileo idot:     " << static_cast<double>(d) << " \n";
}
}  // namespace std
