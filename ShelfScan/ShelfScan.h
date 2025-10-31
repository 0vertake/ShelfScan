#pragma once
#include <string>
#include <vector>
#include <tbb/concurrent_unordered_set.h>
#include "BookData.h"
#include "HttpDownloader.h"
#include "HtmlParser.h"
#include "DataAnalyzer.h"
#include "FileWriter.h"

class ShelfScan {
private:
    HttpDownloader downloader_;
    HtmlParser parser_;
    DataAnalyzer analyzer_;
    FileWriter writer_;
    tbb::concurrent_vector<BookData> scrapedBooks_;
    tbb::concurrent_unordered_set<string> visitedUrls_;
    tbb::concurrent_unordered_set<std::string> seenTitles_;
    ScrapingStats stats_;

public:
    ShelfScan();
    ~ShelfScan();

    void scrapeWithPipeline(const vector<string>& urls);
    vector<string> autoDiscoverUrls(const string& base_url);
    void printStatistics() const;
    void saveResults(const string& filename);
    void reset();
};
