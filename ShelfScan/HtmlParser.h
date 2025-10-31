#pragma once
#include <string>
#include <vector>
#include <gumbo.h>
#include "BookData.h"

using namespace std;

class HtmlParser {
public:
    vector<BookData> parseBooksFromHtml(const string& html_content);
    vector<string> extractPageLinks(const string& html_content);

private:
    string getTextContent(GumboNode* node);
    GumboNode* findNodeByClass(GumboNode* node, const string& class_name);
    GumboNode* findNodeByTag(GumboNode* node, GumboTag tag);

    BookData parseBookFromNode(GumboNode* article_node);
    void searchForBooks(GumboNode* node, vector<BookData>& books);
    void searchForLinks(GumboNode* node, vector<string>& links);

    string cleanText(const string& text);
    float parsePriceString(const string& price_text);
    int parseStarRating(const string& rating_class);
};