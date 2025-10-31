#include "DataAnalyzer.h"
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <numeric>

using namespace std;
using namespace tbb;

AnalysisResults DataAnalyzer::analyzeData(const concurrent_vector<BookData>& books) {
    AnalysisResults results;

    results.fiveStarBooks = countFiveStarBooks(books);
    results.averagePrice = calculateAveragePrice(books);
    results.mostExpensiveBook = findMostExpensiveBook(books);
    results.cheapestBook = findCheapestBook(books);
    results.availabilityStats = analyzeAvailability(books);
    results.totalValue = calculateTotalValue(books);
    results.booksInStock = countBooksInStock(books);
    results.averageRating = calculateAverageRating(books);
    results.ratingDistribution = analyzeRatingDistribution(books);

    return results;
}

int DataAnalyzer::countFiveStarBooks(const concurrent_vector<BookData>& books) {
    return parallel_reduce(blocked_range<size_t>(0, books.size()), 0,
        [&](const blocked_range<size_t>& r, int count) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                if (books[i].starRating == 5) {
                    count++;
                }
            }
            return count;
        },
        plus<int>()
    );
}

float DataAnalyzer::calculateAveragePrice(const concurrent_vector<BookData>& books) {
    if (books.empty()) {
        return 0;
    }

    float total_price = parallel_reduce(blocked_range<size_t>(0, books.size()), 0,
        [&](const blocked_range<size_t>& r, float sum) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                sum += books[i].price;
            }
            return sum;
        },
        plus<float>()
    );

    return total_price / books.size();
}

BookData DataAnalyzer::findMostExpensiveBook(const concurrent_vector<BookData>& books) {
    if (books.empty()) {
        return BookData{};
    }

    return parallel_reduce(blocked_range<size_t>(0, books.size()), books[0],
        [&](const blocked_range<size_t>& r, BookData maxBook) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                if (books[i].price > maxBook.price) {
                    maxBook = books[i];
                }
            }
            return maxBook;
        },
        [](const BookData& a, const BookData& b) {
            if (a.price >= b.price) {
                return a;
            }
            return b;
        }
    );
}

BookData DataAnalyzer::findCheapestBook(const concurrent_vector<BookData>& books) {
    if (books.empty()) {
        return BookData{};
    }

    return parallel_reduce(blocked_range<size_t>(0, books.size()), books[0],
        [&](const blocked_range<size_t>& r, BookData minBook) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                if (books[i].price < minBook.price && books[i].price > 0) {
                    minBook = books[i];
                }
            }
            return minBook;
        },
        [](const BookData& a, const BookData& b) {
            if (a.price <= b.price && a.price > 0) {
                return a;
            }
            return b;
        }
    );
}

int DataAnalyzer::countBooksInStock(const concurrent_vector<BookData>& books) {
    return parallel_reduce(blocked_range<size_t>(0, books.size()), 0,
        [&](const blocked_range<size_t>& r, int count) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                string avail = books[i].availability;

                for (char& ch : avail) {
                    ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
                }

                if (avail.find("in stock") != string::npos) {
                    count++;
                }
            }

            return count;
        },
        plus<int>()
    );
}

map<string, int> DataAnalyzer::analyzeAvailability(const concurrent_vector<BookData>& books) {
    concurrent_unordered_map<string, atomic<int>> availabilityCount;

    parallel_for_each(books.begin(), books.end(),
        [&](const BookData& book) {
            if (!book.availability.empty()) {
                availabilityCount[book.availability]++;
            }
        }
    );

    map<string, int> result;
    for (const auto& pair : availabilityCount) {
        result[pair.first] = pair.second.load();
    }
    return result;
}

float DataAnalyzer::calculateTotalValue(const concurrent_vector<BookData>& books) {
    return parallel_reduce(blocked_range<size_t>(0, books.size()), 0,
        [&](const blocked_range<size_t>& r, float sum) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                sum += books[i].price;
            }
            return sum;
        },
        plus<float>()
    );
}

float DataAnalyzer::calculateAverageRating(const concurrent_vector<BookData>& books) {
    if (books.empty()) {
        return 0;
    }

    float total_rating = parallel_reduce(blocked_range<size_t>(0, books.size()), 0,
        [&](const blocked_range<size_t>& r, float sum) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                sum += books[i].starRating;
            }
            return sum;
        },
        plus<float>()
    );

    return total_rating / books.size();
}

map<int, int> DataAnalyzer::analyzeRatingDistribution(const concurrent_vector<BookData>& books) {
    concurrent_unordered_map<int, atomic<int>> ratingCounts;

    parallel_for_each(books.begin(), books.end(),
        [&](const BookData& book) {
            ratingCounts[book.starRating]++;
        }
    );

    map<int, int> result;
    for (const auto& pair : ratingCounts) {
        result[pair.first] = pair.second.load();
    }

    return result;
}