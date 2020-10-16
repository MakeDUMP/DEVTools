#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <curl/curl.h>
#include <filesystem>
#include <logger.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

        // find all not empty packages
        // for (auto const& pkg : ini.get_child(repo_name)) {
        //     auto const& pkg_name = pkg.first;
        //     auto pkg_files = pkg.second.get_value(std::string {});
        //     if (pkg_name.empty() || pkg_files.empty())
        //         continue;
        //     logger.println("{} -> {}", pkg_name, pkg_files);
        // }
    }

    // https://github.com/nmoinvaz/minizip
    // https://github.com/rxi/microtar

    // mingw-w64-x86_64-*-\\d

    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    makedump::logger {}.println("{red+}: {}", "ERROR", e.what());
    return EXIT_FAILURE;
}
