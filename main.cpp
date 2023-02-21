#include <cstdio>
#include <fstream>
#include <json.hpp>
#include <filesystem>
#include <common.hpp>
#include <HttpClient.h>
#include <regex>
#include <thread>
#include <chrono>


typedef struct Package {
    std::string name = {};
    std::string version = {};
    nlohmann::json config = {};
} Package;

void printUsage() {
    printf("Usage:\n ucomposer --file <path-to-composer.json>\n");
    exit(0);
}

std::vector<std::string> getArgs(int argc, char **argv) {
    std::vector<std::string> result = {};

    for (int i = 0; i < argc; ++i) {
        result.emplace_back(argv[i]);
    }

    return result;
}

bool isUpdatable(const std::string &packageName) {
    std::vector<std::string> excluded = {
            "php",
            "ext-*"
    };

    for (auto &rule: excluded) {
        bool isWildCard = rule.find("*") != std::string::npos;

        if (!isWildCard && packageName == rule) {
            return false;
        }

        if (isWildCard) {
            auto modified = std::regex_replace(rule, std::regex("\\*"), "");

            if (packageName.rfind(modified) == 0) {
                return false;
            }
        }
    }

    return true;
}


int main(int argc, char **argv) {
    curl_global_init(CURL_GLOBAL_ALL);

    if (argc <= 2) {
        printUsage();
    }

    auto cmdArguments = getArgs(argc, argv);

    if (cmdArguments[1] != "--file") {
        printUsage();
    }

    std::string canonical = {};
    std::string targetComposerJson = {};

    try {
        canonical = std::filesystem::canonical(cmdArguments[2]);
    } catch (std::filesystem::filesystem_error &e) {
        errorExit(
                "File composer.json or folder containing composer.json not found at " + cmdArguments[2]);
    }

    if (std::filesystem::is_directory(canonical)) {
        targetComposerJson = canonical + "/composer.json";
    } else {
        targetComposerJson = canonical;
    }

    if (!std::filesystem::is_regular_file(targetComposerJson)) {
        errorExit(targetComposerJson + " not found");
    }


    success("Parsing " + targetComposerJson);

    std::ifstream ifs(targetComposerJson);
    auto packageJson = nlohmann::json::parse(ifs);

    if (!packageJson.contains("require") && !packageJson.contains("require-dev")) {
        errorExit("require or require-dev not found");
    }

    std::atomic<int> downloaded = 0;
    std::vector<Package> packages = {};

    for (auto &item: packageJson["require"].items()) {
        auto key = item.key();
        auto value = item.value();

        if (!isUpdatable(key)) {
            success("Skipping " + key);
            continue;
        }

        Package p;
        p.name = key;
        p.version = value;

        packages.emplace_back(p);
    }

    for (auto &p: packages) {
        std::thread t([&]() {
            HttpClient local;

            std::string packageUrl = "https://repo.packagist.org/p2/" + p.name + ".json";
            auto response = local.getJson(packageUrl);

            if (response.has_value()) {
                p.config = response.value();
            }

            downloaded++;
        });

        t.detach();
    }

    while (downloaded != packages.size()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    sort(packages.begin(), packages.end(), [](const Package &lhs, const Package  &rhs){
        return lhs.name <= rhs.name;
    });

    for (auto &package: packages) {
        if (package.config["packages"].empty() || package.config["packages"][package.name].empty()) {
            continue;
        }

        if (!package.config["packages"][package.name].is_array()) {
            continue;
        }

        auto versions = package.config["packages"][package.name];
        std::string latest = {};

        if (versions.is_array()) {
            auto first = versions[0];

            if (first.contains("version")) {
                latest = first["version"];
            }
        }

        if (!latest.empty()) {
            printf("%-50s %-12s now %-12s\n", package.name.c_str(), latest.c_str(), package.version.c_str());
        } else {
            printf("Package %s invalid\n", package.name.c_str());
        }
    }

    curl_global_cleanup();
    return 0;
}