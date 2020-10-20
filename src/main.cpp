#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <curl/curl.h>
#include <filesystem>
#include <logger.hpp>
#include <memory>
#include <microtar.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <zlib.h>

///
/// Download any file from the Internet
///
auto get_file(std::string const& url) -> std::vector<uint8_t>
{
    std::shared_ptr<CURL> curl { curl_easy_init(), curl_easy_cleanup };
    if (curl.get() == nullptr)
        throw std::runtime_error("Not make `curl` object");

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str()); // download page URL
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);

    std::string error_str {}; // error string buffer
    error_str.reserve(CURL_ERROR_SIZE);
    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, error_str.data());

    std::vector<uint8_t> buffer {}; // data buffer
    auto write_callback = [](void* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        std::vector<uint8_t>& buffer = *static_cast<std::vector<uint8_t>*>(userdata);
        auto realsize = size * nmemb;
        buffer.insert(buffer.end(), reinterpret_cast<uint8_t const*>(ptr), reinterpret_cast<uint8_t const*>(ptr) + realsize);
        return realsize;
    };
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, static_cast<size_t (*)(void*, size_t, size_t, void*)>(write_callback));
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);

    auto result = curl_easy_perform(curl.get()); // get file
    if (result != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(result));
    return buffer;
}

///
/// Main function
///
auto main() -> int
try {
    makedump::logger logger { makedump::logger::format("{white+}", ">>>") };
    logger.println("Curl Version: {yellow}", curl_version());

    //reading from ini file
    boost::property_tree::ptree ini;
    boost::property_tree::read_ini("../settings/minimal.ini", ini);

    // find all not empty repositories
    for (auto const& repo : ini.get_child("Repositories")) {
        auto const& repo_name = repo.first;
        auto repo_url = repo.second.get_value(std::string {});
        if (repo_name.empty() || repo_url.empty())
            continue;

        logger.print("Get database for {yellow+} repository ...", repo_name);
        auto db_tar_gz = get_file(repo_url + '/' + repo_name + ".db.tar.gz");
        logger.println("{yellow+} bytes", db_tar_gz.size());

        logger.print("Unzip database to");
        auto db_tar = std::vector<uint8_t>(4 << 20); // result array in MBytes
        z_stream zstream {};
        zstream.next_in = db_tar_gz.data(); // input char array
        zstream.avail_in = db_tar_gz.size(); // size of input
        zstream.next_out = db_tar.data(); // output char array
        zstream.avail_out = db_tar.capacity(); // size of output

        // Add 32 to windowBits to enable zlib and gzip decoding with automatic
        // header detection, or add 16 to decode only the gzip format.
        // Magic number 32 = enable zlib and gzip decoding with automatic header detection
        auto result = inflateInit2(&zstream, 32);
        if (result != Z_OK)
            throw std::runtime_error("ZLIB open error");

        result = inflate(&zstream, Z_FINISH);
        if (result != Z_STREAM_END)
            throw std::runtime_error { "ZLIB " + std::string(zstream.msg) };
        logger.println("{yellow+} bytes", zstream.total_out);

        auto file_names = std::vector<std::string> {};
        auto tar = mtar_t {};
        auto tar_hdr = mtar_header_t {};
        tar.stream = db_tar.data();
        tar.seek = [](mtar_t*, unsigned) -> int { return MTAR_ESUCCESS; };
        tar.close = [](mtar_t*) -> int { return MTAR_ESUCCESS; };
        tar.read = [](mtar_t* tar, void* data, unsigned size) -> int {
            std::memcpy(data, reinterpret_cast<uint8_t*>(tar->stream) + tar->pos, size);
            return MTAR_ESUCCESS;
        };
        while ((mtar_read_header(&tar, &tar_hdr)) != MTAR_ENULLRECORD) {
            file_names.emplace_back(tar_hdr.name);
            mtar_next(&tar);
        }

        for (auto value = file_names.begin(); value < file_names.begin() + 4; value++)
            logger.println("{}", *value);

        // mtar_find(&tar, "test.txt", &h);
        // p = calloc(1, h.size + 1);
        // mtar_read_data(&tar, p, h.size);
        // printf("%s", p);
        // free(p);

        // find all not empty packages
        // for (auto const& pkg : ini.get_child(repo_name)) {
        //     auto const& pkg_name = pkg.first;
        //     auto pkg_files = pkg.second.get_value(std::string {});
        //     if (pkg_name.empty() || pkg_files.empty())
        //         continue;
        //     logger.println("{} -> {}", pkg_name, pkg_files);
        // }
    }

    // mingw-w64-x86_64-*-\\d

    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    makedump::logger {}.println("{red+}: {}", "ERROR", e.what());
    return EXIT_FAILURE;
}
