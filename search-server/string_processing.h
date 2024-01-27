#pragma once

#include "search_server.h"

#include <iostream>
#include <set>
#include <vector>

{
    using namespace std;
    
    string ReadLine();
    int ReadLineWithNumber();
    
    vector<string> SplitIntoWords(const string& text);
    vector<string> SplitIntoWordsNoStop(const string& text, const SearchServer& search_server);
    
    bool IsValidWord(const string& word);
    bool IsStopWord(const string& word, const SearchServer& search_server);
    
    template <typename StringContainer>
    set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
        set<string> non_empty_strings;
        for (const string& str : strings) {
            if (!IsValidWord(str)) {
                throw invalid_argument("stop words contain illegal characters"s);
            }
            
            if (!str.empty()) {
                non_empty_strings.insert(str);
            }
        }
        return non_empty_strings;
    }
}
