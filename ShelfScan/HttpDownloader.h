#pragma once
#include <string>

class HttpDownloader {
private:
    static const int MAX_RETRIES = 3;
    static const int MAX_TIME = 10;

public:
    HttpDownloader();
    ~HttpDownloader();

    std::string download(const std::string& url);
    std::string downloadWithRetry(const std::string& url, int max_retries = MAX_RETRIES);

private:
    void exponentialBackoff(int attempt);
    bool isValidResponse(const std::string& content);
};