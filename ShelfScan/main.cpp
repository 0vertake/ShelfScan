#include "ShelfScan.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    try {
        ShelfScan scraper;

        vector<string> urls = scraper.autoDiscoverUrls("http://books.toscrape.com/index.html");
        scraper.scrapeWithPipeline(urls);
        scraper.saveResults("results");

        cout << "Scraping successful! Results are saved in results.txt and results.json\n";

    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}