#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#include "curl.hpp"

#include "archive.hpp"
#include <logger.hpp>

///
/// Main function
///
auto main() -> int
try {
    makedump::logger logger { makedump::logger::format("{white+}", ">>>") };
    logger.println("Curl Version: {yellow}", curl::get_version());

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
        auto db_tar_gz = curl::get_file(repo_url + '/' + repo_name + ".db.tar.gz");
        logger.print("{green} ->", db_tar_gz.size());
        auto db_tar = archive::gzip_unpack(db_tar_gz);
        logger.println("{green+} bytes", db_tar.size());

        auto pkg_names = archive::tar_get_files(db_tar);

        // find all not empty packages
        std::regex file_regex {};
        for (auto const& pkg : ini.get_child(repo_name)) {
            auto const& pkg_name = pkg.first;
            auto pkg_files = pkg.second.get_value(std::string {});
            if (pkg_name.empty() || pkg_files.empty())
                continue;
            file_regex = ((repo_name == "mingw64") ? "mingw-w64-x86_64-" : "") + pkg_name + ".*";

            auto found = std::find_if(pkg_names.cbegin(), pkg_names.cend(), [&file_regex, &logger](std::string const& value) {
                return std::regex_match(value, file_regex);
            });
            if (found == pkg_names.cend())
                throw std::runtime_error("Not found `" + pkg_name + "` file in database");
            else
                logger.println("Found {blue+} file in database", *found);
        }
    }

    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    makedump::logger {}.println("{red+}: {}", "ERROR", e.what());
    return EXIT_FAILURE;
}

// mtar_find(&tar, "test.txt", &h);
// p = calloc(1, h.size + 1);
// mtar_read_data(&tar, p, h.size);
// printf("%s", p);
// free(p);
