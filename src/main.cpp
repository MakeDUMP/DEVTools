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

        auto pkg_names = archive::tar_get_file_list(db_tar);

        // find all not empty packages
        for (auto const& pkg : ini.get_child(repo_name)) {
            auto const& pkg_name = pkg.first;
            auto pkg_files = pkg.second.get_value(std::string {});
            if (pkg_name.empty() || pkg_files.empty()) // skip empty
                continue;

            // find package name in database of repository
            auto found = std::find_if(pkg_names.cbegin(), pkg_names.cend(), [&repo_name, &pkg_name](std::string const& value) {
                return std::regex_match(value, std::regex { ((repo_name == "mingw64") ? "mingw-w64-x86_64-" : "") + pkg_name + ".*/desc" });
            });
            if (found == pkg_names.cend())
                throw std::runtime_error("Not found `" + pkg_name + "` file in database");

            // get a description of the found package
            auto pkg_desc_raw = archive::tar_get_file(db_tar, *found);
            auto pkg_desc = std::string(pkg_desc_raw.cbegin(), pkg_desc_raw.cend());

            // get the full name of the found package
            std::smatch match;
            if (std::regex_search(pkg_desc, match, std::regex { "%FILENAME%\n(.*)\n" }) == false)
                throw std::runtime_error("Not found `" + pkg_name + "` file name in descriptor file");
            auto pkg_file_name = match.str(1);
            logger.println("Found package {blue+} in database", pkg_file_name);
        }
    }

    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    makedump::logger {}.println("{red+}: {}", "ERROR", e.what());
    return EXIT_FAILURE;
}
