#pragma once
#include <cstring>
#include <microtar.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <zlib.h>

namespace archive {

///
/// Unpack GZIP byte array
///
inline auto gzip_unpack(std::vector<uint8_t>& raw_gzip) -> std::vector<uint8_t>
{
    z_stream zstream {};

    // Add 32 to windowBits to enable zlib and gzip decoding with automatic
    // header detection, or add 16 to decode only the gzip format.
    // Magic number 32 = enable zlib and gzip decoding with automatic header detection
    auto z_result = inflateInit2(&zstream, 32);
    if (z_result != Z_OK)
        throw std::runtime_error("GZIP inflate init error");

    zstream.next_in = raw_gzip.data(); // input byte array
    zstream.avail_in = raw_gzip.size(); // size of input

    constexpr size_t block_size = 1 << 20; // 1 Mb
    auto buffer = std::array<uint8_t, block_size> {};
    auto result = std::vector<uint8_t> {}; // result array
    do {
        zstream.next_out = buffer.data(); // output byte array
        zstream.avail_out = buffer.size(); // size of output
        z_result = inflate(&zstream, Z_SYNC_FLUSH);
        result.insert(result.cend(), buffer.cbegin(), buffer.cend());
    } while (z_result == Z_OK);
    inflateEnd(&zstream);

    if (z_result != Z_STREAM_END)
        throw std::runtime_error { "GZIP " + std::string(zstream.msg) };
    return result;
}

///
/// Get file list from TAR byte array
///
inline auto tar_get_file_list(std::vector<uint8_t>& raw_tar) -> std::vector<std::string>
{
    auto tar = mtar_t {};
    auto tar_hdr = mtar_header_t {};

    tar.stream = raw_tar.data();
    tar.seek = [](mtar_t*, unsigned) -> int { return MTAR_ESUCCESS; };
    tar.close = [](mtar_t*) -> int { return MTAR_ESUCCESS; };
    tar.read = [](mtar_t* tar, void* data, unsigned size) -> int {
        std::memcpy(data, reinterpret_cast<uint8_t*>(tar->stream) + tar->pos, size);
        return MTAR_ESUCCESS;
    };

    auto result = std::vector<std::string> {};
    while ((mtar_read_header(&tar, &tar_hdr)) != MTAR_ENULLRECORD) {
        result.emplace_back(tar_hdr.name);
        mtar_next(&tar);
    }

    return result;
}

///
/// Get file list from TAR byte array
///
inline auto tar_get_file(std::vector<uint8_t>& raw_tar, std::string const& name) -> std::vector<uint8_t>
{
    auto tar = mtar_t {};
    auto tar_hdr = mtar_header_t {};

    tar.stream = raw_tar.data();
    tar.seek = [](mtar_t*, unsigned) -> int { return MTAR_ESUCCESS; };
    tar.close = [](mtar_t*) -> int { return MTAR_ESUCCESS; };
    tar.read = [](mtar_t* tar, void* data, unsigned size) -> int {
        std::memcpy(data, reinterpret_cast<uint8_t*>(tar->stream) + tar->pos, size);
        return MTAR_ESUCCESS;
    };

    mtar_find(&tar, name.c_str(), &tar_hdr);
    auto result = std::vector<uint8_t>(tar_hdr.size);
    mtar_read_data(&tar, result.data(), tar_hdr.size);

    return result;
}

} // namespace archive
