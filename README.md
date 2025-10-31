# 📚 ShelfScan — Parallel Web Scraper & Analyzer (C++ / TBB)

![C++](https://img.shields.io/badge/C%2B%2B-14%2B-blue?logo=c%2B%2B&logoColor=white)
![Intel TBB](https://img.shields.io/badge/Intel-TBB-lightgrey?logo=intel&logoColor=white)
![libcurl](https://img.shields.io/badge/libcurl-HTTP%20client-orange)
![Gumbo](https://img.shields.io/badge/Gumbo-HTML5%20parser-green)
![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11-lightblue)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)

---

**ShelfScan** is a high-performance web scraping and analysis tool built in **C++**, designed to collect and analyze online book data using **Intel TBB** for parallelization.  
Developed as part of an educational and research project at the **Faculty of Technical Sciences, University of Novi Sad**, it demonstrates modern software design, concurrency, and data-processing practices.

---

## 🧩 System Overview

```text
Main Program
└── ShelfScan (Orchestrator)
    ├── HttpDownloader  - HTTP requests
    ├── HtmlParser      - HTML parsing
    ├── DataAnalyzer    - Statistical analysis
    └── FileWriter      - Output generation
```

### 🔧 Components

| Component | Description |
|------------|-------------|
| **ShelfScan** | Main controller implementing TBB parallel pipeline and task groups |
| **HttpDownloader** | Handles HTTP requests using libcurl with retry logic |
| **HtmlParser** | Parses HTML using Gumbo parser to extract book information |
| **DataAnalyzer** | Performs parallel statistical analysis using TBB reduction algorithms |
| **FileWriter** | Exports results to JSON and formatted text files |

---

## ⚙️ Prerequisites

### Dependencies
- **C++14+ compiler** – MSVC 14.0+ (Visual Studio 2015 or later)
- **Intel TBB** – Threading Building Blocks 2022.2+
- **libcurl** – HTTP requests (8.0+)
- **Gumbo Parser** – HTML5 parsing library

### Recommended Environment
- **OS:** Windows 10 / 11  
- **IDE:** Visual Studio 2022  
- **Package Manager:** vcpkg  

---

## 🚀 Installation

### Option 1 — Using vcpkg (Recommended)

```bash
# 1. Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 2. Install dependencies
.\vcpkg install curl:x64-windows
.\vcpkg install tbb:x64-windows
.\vcpkg install gumbo:x64-windows


# 3. Clone and build ShelfScan
git clone https://github.com/0vertake/ShelfScan.git
cd ShelfScan
```

### Option 2 — Manual Setup
1. Install Intel TBB, libcurl, and Gumbo manually  
2. Configure include / library paths in Visual Studio  
3. Build the solution  

---

## 🧠 Usage

Run the compiled executable:

```bash
ShelfScan.exe
```

The application will:
1. Auto-discover book catalog pages (up to 50)  
2. Download and parse pages in parallel  
3. Perform data analysis  
4. Export results to `results.txt` and `results.json`

### Configuration
Edit constants in `ShelfScan.cpp`:

```cpp
const int MAX_PAGES = 50;
const size_t PIPELINE_TOKENS = std::thread::hardware_concurrency() * 2;
const int DISCOVERY_GROUP_WORKERS = std::thread::hardware_concurrency();
```

---

## 📊 Output Examples

### `results.json`
```json
[
  {
    "title": "Hold Your Breath (Search and Rescue #1)",
    "price": 28.82,
    "starRating": 1,
    "availability": "In stock",
    "imageUrl": "http://books.toscrape.com/../media/cache/0b/89/0b89c3b317d0f89da48356a0b5959c1e.jpg"
  },
  {
    "title": "Hamilton: The Revolution",
    "price": 58.79,
    "starRating": 3,
    "availability": "In stock",
    "imageUrl": "http://books.toscrape.com/../media/cache/34/ef/34ef0844cb1fbca6ab73444087fcf0e6.jpg"
  },
  {
    "title": "Greek Mythic History",
    "price": 10.23,
    "starRating": 5,
    "availability": "In stock",
    "imageUrl": "http://books.toscrape.com/../media/cache/36/cf/36cf56c7bdf35aadbcc6f05a8e8d8fcb.jpg"
  }
]
```

### `results.txt`
```
===============================================
        WEB SCRAPER - ANALYSIS RESULTS        
===============================================

PERFORMANCE STATS:
- Pages processed: 50
- Books found: 1000
- Failed requests: 0
- Execution time: 4.657s

CONTENT ANALYSIS:
1. Number of 5-star books: 196
2. Average book price: £34.82
3. Most expensive book: "The Perfect Play (Play by Play #1)" (£59.99)
4. Cheapest book: "An Abundance of Katherines" (£10.00)
5. Total value of all books: £34818.00

ADDITIONAL STATS:
- Average rating: 2.9/5
- Books in stock: 1000

RATING DISTRIBUTION:
- 1 star: 226 books
- 2 star: 196 books
- 3 star: 203 books
- 4 star: 179 books
- 5 star: 196 books

AVAILABILITY:
- In stock: 1000 books

===============================================

```

---

## 🧮 Technical Highlights

### Parallelization Strategy
- **URL Discovery:** TBB `task_group` with concurrent workers  
- **Scraping Pipeline:** 3-stage TBB `parallel_pipeline`  
  - Stage 1 — URL generation (serial)  
  - Stage 2 — HTTP downloads (parallel)  
  - Stage 3 — HTML parsing (parallel)  
- **Data Analysis:** TBB `parallel_reduce` for aggregation  

### Thread Safety
- `tbb::concurrent_vector` — stores scraped books  
- `tbb::concurrent_unordered_set` — tracks visited URLs  
- Atomic counters for stats tracking  

### Error Handling
- Exponential backoff (max 3 retries)  
- Response validation & safe parsing  
- Exception safety across all stages  

---

## ⚡ Performance Snapshot

| Metric | Value (8-core CPU) |
|--------|--------------------|
| Pages per second | 10 - 12 |
| Books per second | 200 - 240 |
| Total time (50 pages) | 4 - 5 seconds |
| Scalability | Linear with core count |


---

## 🗂️ Project Structure
```text
ShelfScan/
├── main.cpp
├── ShelfScan.h/.cpp
├── HttpDownloader.h/.cpp
├── HtmlParser.h/.cpp
├── DataAnalyzer.h/.cpp
├── FileWriter.h/.cpp
├── BookData.h
├── ScrapingStats.h
└── README.md
```

---

## 🎓 Educational Purpose

This project demonstrates:
- **Parallel Programming** (Intel TBB)  
- **Network Programming** (libcurl)  
- **HTML Parsing** (Gumbo)  
- **Data Analysis & Aggregation**  
- **Modular Software Design**  
- **Exception Safety & Resilience**  
- **Performance Optimization**

---

## 🪪 License & Acknowledgments

This project is for **educational and non-commercial use**.  
Please respect `robots.txt` and website terms when scraping data.

**Acknowledgments**
- [Books to Scrape](https://books.toscrape.com/) — practice dataset  
- [Intel TBB](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html)  
- [libcurl](https://curl.se/libcurl/)  
- [Gumbo Parser](https://github.com/google/gumbo-parser)

---

⭐ *If you enjoyed this project, consider giving it a star — it really helps!*  
