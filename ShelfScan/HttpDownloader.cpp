#include "HttpDownloader.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <curl/curl.h>

using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

HttpDownloader::HttpDownloader() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpDownloader::~HttpDownloader() {
    curl_global_cleanup();
}

string HttpDownloader::download(const string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw runtime_error("Failed to initialize libcurl");
    }

    string response_data;
    CURLcode res;

    try {

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);          // Follow redirects
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);              // Max 10 redirects
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);          // 5 seconds for connection
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, MAX_TIME);           // Max time for whole request
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);             // Fail on HTTP errors

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw runtime_error("HTTP request failed: " + string(curl_easy_strerror(res)));
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (response_code >= 400) {
            curl_easy_cleanup(curl);
            throw runtime_error("HTTP error " + to_string(response_code) + " for URL: " + url);
        }

        curl_easy_cleanup(curl);

        if (!isValidResponse(response_data)) {
            throw runtime_error("Invalid HTTP response received from: " + url);
        }

        return response_data;

    }
    catch (...) {
        curl_easy_cleanup(curl);
        throw;
    }
}

string HttpDownloader::downloadWithRetry(const string& url, int maxRetries) {
    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        try {
            cout << "Attempting download " << attempt << "/" << maxRetries << " for: " << url << endl;
            return download(url);
        }
        catch (const exception& e) {
            cerr << "Download attempt " << attempt << " failed for " << url << ": " << e.what() << endl;

            if (attempt == maxRetries) {
                throw runtime_error("All " + to_string(maxRetries) + " download attempts failed for: " + url);
            }

            exponentialBackoff(attempt);
        }
    }
    return "";
}

void HttpDownloader::exponentialBackoff(int attempt) {
    int waitTimeMs = (1 << attempt) * 1000;
    cout << "Waiting " << waitTimeMs / 1000.0 << "s before retry..." << endl;
    this_thread::sleep_for(chrono::milliseconds(waitTimeMs));
}

bool HttpDownloader::isValidResponse(const string& content) {
    if (content.empty()) return false;

    if (content.find("<!DOCTYPE html>") == string::npos && content.find("<html") == string::npos) {
        return false;
    }

    if (content.find("404 Not Found") != string::npos || content.find("500 Internal Server Error") != string::npos || content.find("403 Forbidden") != string::npos) {
        return false;
    }

    return true;
}