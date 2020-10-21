#pragma once
#include <curl/curl.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace curl {

///
/// Get string of version
///
inline auto get_version() -> std::string
{
    return curl_version();
}

///
/// Download any file from the Internet
///
inline auto get_file(std::string const& url) -> std::vector<uint8_t>
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

} // namespace curl
