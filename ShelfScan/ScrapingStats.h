#pragma once

#include <atomic>
#include <chrono>

using namespace std;

struct ScrapingStats {
    atomic<int> pagesProcessed{ 0 };
    atomic<int> booksFound{ 0 };
    atomic<int> failedRequests{ 0 };
    chrono::steady_clock::time_point startTime;
    chrono::steady_clock::time_point endTime;
};