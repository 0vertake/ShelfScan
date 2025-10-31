#include "ShelfScan.h"
#include <iostream>
#include <iomanip>

#include <tbb/parallel_pipeline.h>
#include <tbb/task_group.h>
#include <tbb/concurrent_queue.h>

using namespace chrono;

const size_t PIPELINE_TOKENS = max<unsigned int>(2, thread::hardware_concurrency() * 2);
const int DISCOVERY_GROUP_WORKERS = max<unsigned int>(2, thread::hardware_concurrency());
const int MAX_PAGES = 50;

ShelfScan::ShelfScan() {
    cout << "ShelfScan initialized." << endl;
    cout << "Cores available: " << thread::hardware_concurrency() << endl;
}

ShelfScan::~ShelfScan() {
    cout << "ShelfScan finished." << endl;
}

// TBB Pipeline divided into 3 stages:
// (1) Generate URLs
// (2) Download pages
// (3) Parse & store results
void ShelfScan::scrapeWithPipeline(const vector<string>& urls) {
    cout << "Starting parallel scraping for " << urls.size() << " URL(s)." << endl;

    stats_.startTime = steady_clock::now();

    atomic<size_t> url_index{ 0 };

    tbb::parallel_pipeline(PIPELINE_TOKENS, tbb::make_filter<void, string>(
        tbb::filter_mode::serial_in_order,
        // Stage 1: Get next URL
        [&](tbb::flow_control& fc) -> string {
            size_t index = url_index.fetch_add(1);
            if (index >= urls.size()) {
                fc.stop();  // stop pipeline when all URLs processed
                return "";
            }
            cout << "Pipeline: Generating URL " << (index + 1) << "/" << urls.size() << "\n";
            return urls[index];
        }
    ) &

        // Stage 2: Download HTML content from URL
        tbb::make_filter<string, pair<string, string>>(tbb::filter_mode::parallel,
            [this](string url) -> pair<string, string> {
                try {
                    if (visitedUrls_.count(url) > 0) {
                        return make_pair(url, "");  // skip already visited
                    }

                    visitedUrls_.insert(url);

                    // Download page
                    cout << "Pipeline: Downloading " << url << "\n";
                    string html_content = downloader_.downloadWithRetry(url);
                    stats_.pagesProcessed++;

                    return make_pair(url, html_content);

                }
                catch (const exception& e) {
                    stats_.failedRequests++;
                    cerr << "Pipeline download error for " << url << ": " << e.what() << endl;
                    return make_pair(url, "");
                }
            }
        ) &

        // Stage 3: Parse HTML content and store extracted books
        tbb::make_filter<pair<string, string>, void>(tbb::filter_mode::parallel, [this](pair<string, string> urlContent) {
            if (urlContent.second.empty()) {
                return;
            }

            try {
                cout << "Pipeline: Parsing " << urlContent.first << endl;

                auto books = parser_.parseBooksFromHtml(urlContent.second);

                for (const auto& book : books) {
                    scrapedBooks_.push_back(book);
                }

                stats_.booksFound += static_cast<int>(books.size());

                cout << "Pipeline: Stored " << books.size() << " books from " << urlContent.first << endl;

            }
            catch (const exception& e) {
                cerr << "Pipeline parse error for " << urlContent.first << ": " << e.what() << endl;
            }
            }
        )
    );

    stats_.endTime = steady_clock::now();

    cout << "Pipeline scraping finished!\n";
    printStatistics();
}

// Auto-discover multiple page URLs starting from a base URL
vector<string> ShelfScan::autoDiscoverUrls(const string& baseUrl) {
    cout << "Auto URL indexing for " << baseUrl << "..." << endl;

    tbb::concurrent_unordered_set<string> discoveredUrls;
    tbb::concurrent_bounded_queue<string> urlsToProcess;

    urlsToProcess.push(baseUrl);

    atomic<int> totalProcessed{ 0 };
    tbb::task_group discoveryGroup;

    // thread::hardware_concurrency() can return 0 if the number of cores is unknown,
    // so in that case it's set to 1
    int numWorkers = DISCOVERY_GROUP_WORKERS;
    if (numWorkers == 0) {
        numWorkers = 1;
    }

    // Launch workers to explore new URLs concurrently
    for (int i = 0; i < numWorkers; ++i) {
        discoveryGroup.run([&, i] {
            try {
                while (true) {
                    string currentUrl;
                    // Worker waits for next url if it's empty
                    urlsToProcess.pop(currentUrl);

                    // Skip if already processed
                    if (discoveredUrls.count(currentUrl) > 0) {
                        continue;
                    }

                    discoveredUrls.insert(currentUrl);
                    int processedNow = ++totalProcessed;

                    cout << "[Discovery Worker " << i << "] Exploring: " << currentUrl
                        << " (" << processedNow << " processed)" << endl;

                    // Download current page & extract links
                    string htmlContent = downloader_.downloadWithRetry(currentUrl);
                    auto newUrls = parser_.extractPageLinks(htmlContent);

                    int newFound = 0;
                    for (const auto& newUrl : newUrls) {
                        // Accept only catalogue or site links
                        if ((newUrl.find("catalogue/page-") != string::npos ||
                            newUrl.find("books.toscrape.com") != string::npos) &&
                            discoveredUrls.count(newUrl) == 0) {
                            urlsToProcess.push(newUrl);
                            newFound++;
                        }
                    }

                    cout << "[Discovery Worker " << i << "] Found " << newFound
                        << " new URLs from " << currentUrl << endl;

                    // Stop discovery when page limit reached
                    if (processedNow >= MAX_PAGES) {
                        cout << "Reached maximum page limit (" << MAX_PAGES << ")" << endl;
                        urlsToProcess.abort();
                        break;
                    }
                }
            }
            catch (const tbb::user_abort&) {
                cout << "[Discovery Worker " << i << "] Aborted (queue closed)" << endl;
            }
            catch (const exception& e) {
                cerr << "Discovery error in worker " << i << ": " << e.what() << endl;
            }

            cout << "[Discovery Worker " << i << "] Finished" << endl;
            });
    }

    discoveryGroup.wait();

    // Convert discovered URLs into normal vector for sorting
    vector<string> result;
    for (const auto& url : discoveredUrls) {
        // Skip index.html since we already processed it during discovery
        if (url.find("index.html") == string::npos) {
            result.push_back(url);
        }
    }

    sort(result.begin(), result.end());
    cout << "Auto discovery completed. Found " << result.size() << " unique URLs." << endl;

    return result;
}


void ShelfScan::printStatistics() const {
    auto duration = duration_cast<milliseconds>(stats_.endTime - stats_.startTime);

    cout << "\n=== PERFORMANCE STATS ===\n";
    cout << "Pages processed: " << stats_.pagesProcessed.load() << "\n";
    cout << "Books found: " << stats_.booksFound.load() << "\n";
    cout << "Failed requests: " << stats_.failedRequests.load() << "\n";
    cout << "Total time: " << duration.count() << " ms\n";

    if (duration.count() > 0) {
        double pagesPerSecond = (stats_.pagesProcessed.load() * 1000.0) / duration.count();
        double booksPerSecond = (stats_.booksFound.load() * 1000.0) / duration.count();

        cout << "Speed: " << fixed << setprecision(2)
            << pagesPerSecond << " pages/s, "
            << booksPerSecond << " books/s\n";
    }

    cout << "Unique URLs: " << visitedUrls_.size() << "\n";
    cout << "================================\n\n";
}

void ShelfScan::saveResults(const string& filename) {

    vector<BookData> booksCopy;

    for (const auto& book : scrapedBooks_) {
        booksCopy.push_back(book);
    }

    auto analysisResults = analyzer_.analyzeData(scrapedBooks_);
    
    // Saves analysis stats to .txt file
    writer_.writeResults(filename, analysisResults, stats_);
    
    // Saves all books to .json file
    writer_.writeRawData(filename, scrapedBooks_);
}
