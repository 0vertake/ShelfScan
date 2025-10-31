#pragma once
#include "BookData.h"
#include <map>
#include <tbb/concurrent_vector.h>

using namespace std;
using namespace tbb;

struct AnalysisResults {
    int fiveStarBooks;
    float averagePrice;
    BookData mostExpensiveBook;
    map<string, int> availabilityStats;
    BookData cheapestBook;
    float totalValue;
    int booksInStock;
    float averageRating;
    map<int, int> ratingDistribution;
};

class DataAnalyzer {
public:
    AnalysisResults analyzeData(const concurrent_vector<BookData>& books);

private:
    int countFiveStarBooks(const concurrent_vector<BookData>& books);
    float calculateAveragePrice(const concurrent_vector<BookData>& books);
    BookData findMostExpensiveBook(const concurrent_vector<BookData>& books);
    BookData findCheapestBook(const concurrent_vector<BookData>& books);
    map<string, int> analyzeAvailability(const concurrent_vector<BookData>& books);
    float calculateTotalValue(const concurrent_vector<BookData>& books);
    int countBooksInStock(const concurrent_vector<BookData>& books);
    float calculateAverageRating(const concurrent_vector<BookData>& books);
    map<int, int> analyzeRatingDistribution(const concurrent_vector<BookData>& books);
};