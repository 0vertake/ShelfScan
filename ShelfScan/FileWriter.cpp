#include "FileWriter.h"
#include "ScrapingStats.h"
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace chrono;

void FileWriter::writeResults(const string& filename, const AnalysisResults& results, const ScrapingStats& stats) {
    ofstream file(filename + ".txt");
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename + ".txt");
    }

    file << formatResults(results, stats);
    file.close();
}

void FileWriter::writeRawData(const string& filename, const tbb::concurrent_vector<BookData>& books) {
    ofstream file(filename + ".json");
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename + ".json");
    }

    file << "[\n";

    for (size_t i = 0; i < books.size(); ++i) {
        const auto& book = books[i];
        file << "  {\n";
        file << "    \"title\": \"" << book.title << "\",\n";
        file << "    \"price\": " << book.price << ",\n";
        file << "    \"starRating\": " << book.starRating << ",\n";
        file << "    \"availability\": \"" << book.availability << "\",\n";
        file << "    \"imageUrl\": \"" << book.imageUrl << "\"\n";
        file << "  }";

        if (i + 1 < books.size()) {
            file << ",";
        }
        file << "\n";
    }

    file << "]\n";

    file.close();
}

string FileWriter::formatResults(const AnalysisResults& results, const ScrapingStats& stats) {
    ostringstream oss;

    oss << "===============================================\n";
    oss << "        WEB SCRAPER - ANALYSIS RESULTS        \n";
    oss << "===============================================\n\n";

    oss << "PERFORMANCE STATS:\n";
    oss << "- Pages processed: " << stats.pagesProcessed.load() << "\n";
    oss << "- Books found: " << stats.booksFound.load() << "\n";
    oss << "- Failed requests: " << stats.failedRequests.load() << "\n";
    oss << "- Execution time: " << formatDuration(stats.startTime, stats.endTime) << "\n\n";

    oss << "CONTENT ANALYSIS:\n";
    oss << "1. Number of 5-star books: " << results.fiveStarBooks << "\n";
    oss << "2. Average book price: £" << fixed << setprecision(2)
        << results.averagePrice << "\n";
    oss << "3. Most expensive book: \"" << results.mostExpensiveBook.title
        << "\" (£" << results.mostExpensiveBook.price << ")\n";
    oss << "4. Cheapest book: \"" << results.cheapestBook.title
        << "\" (£" << results.cheapestBook.price << ")\n";
    oss << "5. Total value of all books: £" << fixed << setprecision(2)
        << results.totalValue << "\n\n";

    oss << "ADDITIONAL STATS:\n";
    oss << "- Average rating: " << fixed << setprecision(1)
        << results.averageRating << "/5\n";
    oss << "- Books in stock: " << results.booksInStock << "\n\n";

    oss << "RATING DISTRIBUTION:\n";
    for (const auto& pair : results.ratingDistribution) {
        oss << "- " << pair.first << " star: " << pair.second << " books\n";
    }
    oss << "\n";

    oss << "AVAILABILITY:\n";
    for (const auto& pair : results.availabilityStats) {
        oss << "- " << pair.first << ": " << pair.second << " books\n";
    }

    oss << "\n===============================================\n";

    return oss.str();
}

string FileWriter::formatDuration(const steady_clock::time_point& start, const steady_clock::time_point& end) {
    auto duration = duration_cast<milliseconds>(end - start);
    auto seconds = duration_cast<chrono::seconds>(duration);
    auto milliseconds = duration - seconds;

    int ms = milliseconds.count();
    string msStr = to_string(ms);

    if (ms < 10) {
        msStr = "00" + msStr;
    }
    else if (ms < 100) {
        msStr = "0" + msStr;
    }

    return to_string(seconds.count()) + "." + msStr + "s";
}
