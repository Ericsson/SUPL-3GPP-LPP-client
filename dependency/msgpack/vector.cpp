#include "vector.hpp"

namespace msgpack {

template void pack(Packer&, std::vector<bool> const&) NOEXCEPT;
template void pack(Packer&, std::vector<int8_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<int16_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<int32_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<int64_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<uint8_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<uint16_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<uint32_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<uint64_t> const&) NOEXCEPT;
template void pack(Packer&, std::vector<float> const&) NOEXCEPT;
template void pack(Packer&, std::vector<double> const&) NOEXCEPT;
template void pack(Packer&, std::vector<std::string> const&) NOEXCEPT;

template bool unpack(Unpacker&, std::vector<bool>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<int8_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<int16_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<int32_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<int64_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<uint8_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<uint16_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<uint32_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<uint64_t>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<float>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<double>&) NOEXCEPT;
template bool unpack(Unpacker&, std::vector<std::string>&) NOEXCEPT;

}  // namespace msgpack
