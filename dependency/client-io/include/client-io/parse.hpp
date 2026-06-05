#pragma once
#include <io/serial.hpp>
#include <io/stream.hpp>

#include <string>
#include <unordered_map>
#include <vector>

using Options = std::unordered_map<std::string, std::string>;

Options parse_options(std::string const& str);
bool    parse_bool(Options const& options, std::string const& key, bool default_value);

io::BaudRate  parse_baudrate(std::string const& str);
io::DataBits  parse_databits(std::string const& str);
io::StopBits  parse_stopbits(std::string const& str);
io::ParityBit parse_paritybit(std::string const& str);

io::ReadBufferConfig parse_read_config(Options const& options);

std::vector<std::string> parse_list(std::string const& str, char delimiter = '+');

std::string generate_unique_id();
