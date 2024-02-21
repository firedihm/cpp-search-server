#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ' || c == '\t') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    
    return words;
}

vector<string_view> SplitIntoWordsView(string_view str) {
    vector<string_view> result;
    for(size_t caret; (caret = str.find_first_not_of(' ')) != str.npos; ) {
        str.remove_prefix(min(str.size(), caret));
        
        caret = str.find(' ');
        result.push_back(str.substr(0, min(str.size(), caret)));
        
        str.remove_prefix(min(str.size(), caret));
    }
    
    return result;
}
