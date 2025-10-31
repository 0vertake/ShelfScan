#include "HtmlParser.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <gumbo.h>

using namespace std;

string HtmlParser::getTextContent(GumboNode* node) {
    if (node->type == GUMBO_NODE_TEXT) {
        return string(node->v.text.text);

    } else if (node->type == GUMBO_NODE_ELEMENT) {
        string text_content = "";
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            text_content += getTextContent(static_cast<GumboNode*>(children->data[i]));
        }

        return text_content;
    }
    return "";
}

GumboNode* HtmlParser::findNodeByClass(GumboNode* node, const string& class_name) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return nullptr;
    }
    
    GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes, "class");
    if (class_attr) {
        string class_value = string(class_attr->value);
        if (class_value.find(class_name) != string::npos) {
            return node;
        }
    }
    
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode* result = findNodeByClass(static_cast<GumboNode*>(children->data[i]), class_name);
        if (result) {
            return result;
        }
    }
    
    return nullptr;
}

GumboNode* HtmlParser::findNodeByTag(GumboNode* node, GumboTag tag) {
    if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == tag) {
        return node;
    }
    
    if (node->type == GUMBO_NODE_ELEMENT) {
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            GumboNode* result = findNodeByTag(static_cast<GumboNode*>(children->data[i]), tag);
            if (result) {
                return result;
            }
        }
    }
    
    return nullptr;
}

string HtmlParser::cleanText(const string& text) {
    string result = text;
    
    size_t start = result.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos) {
        return "";
    }
    
    size_t end = result.find_last_not_of(" \t\n\r\f\v");
    result = result.substr(start, end - start + 1);
    
    string::size_type pos = 0;
    while ((pos = result.find("  ", pos)) != string::npos) {
        result.replace(pos, 2, " ");
    }
    
    return result;
}

float HtmlParser::parsePriceString(const string& price_text) {
    string cleaned = cleanText(price_text);

    string numbersOnly;
    bool foundDot = false;

    for (unsigned char uc : cleaned) {
        if (uc >= '0' && uc <= '9') {
            numbersOnly += static_cast<char>(uc);
        }
        else if ((uc == '.' || uc == ',') && !foundDot) {
            numbersOnly += '.';
            foundDot = true;
        }
    }

    try {
        if (numbersOnly.empty()) {
            return 0;
        }
        else {
            return stof(numbersOnly);
        }
    }
    catch (...) {
        return 0;
    }
}

int HtmlParser::parseStarRating(const string& ratingClass) {
    string lowerClass = ratingClass;

    for (char& ch : lowerClass) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    if (lowerClass.find("one") != string::npos) {
        return 1;
    }
    if (lowerClass.find("two") != string::npos) {
        return 2;
    }
    if (lowerClass.find("three") != string::npos) {
        return 3;
    }
    if (lowerClass.find("four") != string::npos) {
        return 4;
    }
    if (lowerClass.find("five") != string::npos) {
        return 5;
    }

    return 0;
}

// Parses single book 
BookData HtmlParser::parseBookFromNode(GumboNode* articleNode) {
    BookData book;
    
    // Find title
    GumboNode* h3_node = findNodeByTag(articleNode, GUMBO_TAG_H3);
    if (h3_node) {
        GumboNode* a_node = findNodeByTag(h3_node, GUMBO_TAG_A);
        if (a_node) {
            GumboAttribute* titleAttr = gumbo_get_attribute(&a_node->v.element.attributes, "title");
            if (titleAttr) {
                book.title = cleanText(string(titleAttr->value));
            } else {
                book.title = cleanText(getTextContent(a_node));
            }
        }
    }
    
    // Find price
    GumboNode* priceNode = findNodeByClass(articleNode, "price_color");
    if (priceNode) {
        string priceText = getTextContent(priceNode);
        book.price = parsePriceString(priceText);
    }
    
    // Find rating
    GumboNode* ratingNode = findNodeByClass(articleNode, "star-rating");
    if (ratingNode) {
        GumboAttribute* classAttr = gumbo_get_attribute(&ratingNode->v.element.attributes, "class");
        if (classAttr) {
            book.starRating = parseStarRating(string(classAttr->value));
        }
    }
    
    // Find availability
    GumboNode* availNode = findNodeByClass(articleNode, "availability");
    if (availNode) {
        book.availability = cleanText(getTextContent(availNode));
    }
    
    // Find image URL
    GumboNode* imgNode = findNodeByTag(articleNode, GUMBO_TAG_IMG);
    if (imgNode) {
        GumboAttribute* srcAttr = gumbo_get_attribute(&imgNode->v.element.attributes, "src");
        if (srcAttr) {
            string url = string(srcAttr->value);
            if (url.find("http") != 0) {
                url = "http://books.toscrape.com/" + url;
            }
            book.imageUrl = url;
        }
    }
    
    return book;
}

// Search for all book articles in DOM
void HtmlParser::searchForBooks(GumboNode* node, vector<BookData>& books) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    
    // Looks for <article class="product_pod">
    if (node->v.element.tag == GUMBO_TAG_ARTICLE) {
        GumboAttribute* class_attr = gumbo_get_attribute(&node->v.element.attributes, "class");
        if (class_attr && string(class_attr->value).find("product_pod") != string::npos) {
            BookData book = parseBookFromNode(node);
            if (!book.title.empty()) {
                books.push_back(book);
                cout << "Found book: " << book.title << " (" << book.price << " GBP)" << endl;
            }
        }
    }
    
    // Recursively searches children
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        searchForBooks(static_cast<GumboNode*>(children->data[i]), books);
    }
}

// Search for pagination links
void HtmlParser::searchForLinks(GumboNode* node, vector<string>& links) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    
    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href) {
            string link = string(href->value);
            
            // Filters for pagination links
            if (link.find("page-") != string::npos) {
                // Cleans relative paths
                if (link.substr(0, 2) == "./") {
                    link = link.substr(2);
                }
                
                // Makes absolute URL
                if (link.find("http") != 0) {
                    if (link[0] == '/') {
                        link = "http://books.toscrape.com" + link;
                    } else {
                        if (link.find("catalogue/") == string::npos) {
                            link = "http://books.toscrape.com/catalogue/" + link;
                        } else {
                            link = "http://books.toscrape.com/" + link;
                        }
                    }
                }
                
                // Adds unique links only
                if (link.find("books.toscrape.com") != string::npos && find(links.begin(), links.end(), link) == links.end()) {
                    links.push_back(link);
                    cout << "Found pagination link: " << link << endl;
                }
            }
        }
    }
    
    // Recursively searches children
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        searchForLinks(static_cast<GumboNode*>(children->data[i]), links);
    }
}

// Finds all books on page
vector<BookData> HtmlParser::parseBooksFromHtml(const string& html_content) {
    vector<BookData> books;
    
    GumboOutput* output = gumbo_parse(html_content.c_str());
    searchForBooks(output->root, books);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    
    return books;
}

// Finds all pagination links on page
vector<string> HtmlParser::extractPageLinks(const string& html_content) {
    vector<string> links;
    
    GumboOutput* output = gumbo_parse(html_content.c_str());
    searchForLinks(output->root, links);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    
    cout << "Gumbo parser found " << links.size() << " pagination links" << endl;
    return links;
}