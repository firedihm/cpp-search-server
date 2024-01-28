#pragma once

#include <set>
#include <string>
#include <vector>

namespace std {
    string ReadLine();
    int ReadLineWithNumber();
    
    bool IsValidWord(const string& word);
    vector<string> SplitIntoWords(const string& text);
    
    template <typename StringContainer>
    set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
        set<string> non_empty_strings;
        for (const string& str : strings) {
            if (!str.empty() && IsValidWord(str)) {
                non_empty_strings.insert(str);
            }
        }
        return non_empty_strings;
    }
} //namespace std
