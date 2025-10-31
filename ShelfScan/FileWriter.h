#pragma once
#include "BookData.h"
#include "ScrapingStats.h"
#include "DataAnalyzer.h"
#include <string>
#include <tbb/concurrent_vector.h>

using namespace std;
using namespace chrono;

class FileWriter {
public:
    void writeResults(const string& filename, const AnalysisResults& results, const ScrapingStats& stats);
    void writeRawData(const string& filename, const tbb::concurrent_vector<BookData>& books);

private:
    string formatResults(const AnalysisResults& results, const ScrapingStats& stats);
    string formatDuration(const steady_clock::time_point& start, const steady_clock::time_point& end);
};
