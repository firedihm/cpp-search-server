#include "string_processing.h"

#include <algorithm>
#include <iostream>

namespace std {
    string ReadLine() {
        string s;
        getline(cin, s);
        return s;
    }
    
    int ReadLineWithNumber() {
        int result;
        cin >> result;
        ReadLine();
        return result;
    }
    
    bool IsValidWord(const string& word) {
        return none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
    }
    
    vector<string> SplitIntoWords(const string& text) {
        vector<string> words;
        string word;
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
} //namespace std
