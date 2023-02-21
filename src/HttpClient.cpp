#include "HttpClient.h"
#include <common.hpp>

HttpClient::HttpClient() {

}

HttpClient::~HttpClient() {

}

std::optional<nlohmann::json> HttpClient::getJson(const std::string &path) {
    curl = curl_easy_init();

    std::string body;
    long responseCode = 200;

    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpClient::write);


    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        error(curl_easy_strerror(res));
        return {};
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    if (verbosity >= 1) {
        if (responseCode >= 200 && responseCode < 400) {
            success("GET " + std::to_string(responseCode) + " " + std::to_string(body.size()) + " bytes " + path);
        } else {
            error("GET " + std::to_string(responseCode) + " " + std::to_string(body.size()) + " bytes " + path);
        }
    }

    if (responseCode != 200) {
        return {};
    }

    auto json = nlohmann::json::parse(body);
    curl_easy_cleanup(curl);

    return json;
}

size_t HttpClient::write(void *ptr, size_t size, size_t nmemb, void *userdata) {
    ((std::string *) userdata)->append((char *) ptr, size * nmemb);
    return size * nmemb;
}
