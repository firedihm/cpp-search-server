#include "string_processing.h"

#include <iostream>

{
    using namespace std;
    
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
    
    bool IsStopWord(const string& word, const SearchServer* search_server) {
        return search_server->GetStopWords().count(word) != 0;
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
    
    vector<string> SplitIntoWordsNoStop(const string& text, const SearchServer* search_server) {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (IsValidWord(word) && !IsStopWord(word, search_server)) {
                words.push_back(word);
            }
        }
        return words;
    }
}
