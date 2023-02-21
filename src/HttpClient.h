#pragma once


#include <curl/curl.h>
#include <optional>
#include "json.hpp"

class HttpClient {
private:
    CURL *curl = nullptr;
    CURLcode res = CURLE_OK;

    int verbosity = 1;

    static size_t write(void *contents, size_t size, size_t nmemb, void *userp);
public:
    HttpClient();

    virtual ~HttpClient();

    std::optional<nlohmann::json> getJson(const std::string &path);
};