#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <curl/curl.h>
#include <logger.hpp>
#include <memory>
#include <string>
#include <vector>

///
/// Download any file from the Internet, using CURL library
///
auto get_file(std::shared_ptr<CURL> const& curl, std::string const& url) -> std::vector<uint8_t>
{
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

    // test curl
    logger.println("Curl Version: {yellow}", curl_version());
    std::shared_ptr<CURL> curl { curl_easy_init(), curl_easy_cleanup };
    if (curl.get() == nullptr)
        throw std::runtime_error("Not make `curl` object");
    auto url = "http://repo.msys2.org/msys/x86_64/msys.db.tar.gz";
    auto data = get_file(curl, url);
    logger.println("Get from {green} {yellow} bytes", url, data.size());

    // test reading from ini file
    boost::property_tree::ptree ini;
    boost::property_tree::read_ini("../settings/test.ini", ini);
    logger.println("Section13.Param  <string> = {cyan+}", ini.get<std::string>("Section13.Param", ""));
    logger.println("Section14.Param1 <int>    = {cyan+}", ini.get<int>("Section14.Param1", 0));
    logger.println("Section14.Param2 <double> = {cyan+}", ini.get<double>("Section14.Param2", .0));

    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    makedump::logger {}.println("{red+}: {}", "ERROR", e.what());
    return EXIT_FAILURE;
}
