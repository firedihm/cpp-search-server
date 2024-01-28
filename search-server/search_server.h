#pragma once

#include "document.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace std {
    const int MAX_RESULT_DOCUMENT_COUNT = 5;
    const double EPSILON = 1e-6;
    
    class SearchServer {
    public:
        template <typename StringContainer>
        explicit SearchServer(const StringContainer& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {}
        explicit SearchServer(const string &stop_words) : SearchServer(SplitIntoWords(stop_words)) {}
        
        const set<string>& GetStopWords() const;
        size_t GetDocumentCount() const;
        int GetDocumentId(uint n) const;
        
        bool IsStopWord(const string& word) const;
        
        bool AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings);
        
        tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const;
        
        //вызов без явно переданного статуса или предиката = вызов с предикатом по умолчанию
        vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus sought_status = DocumentStatus::ACTUAL) const {
            return FindTopDocuments(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                                   return status == sought_status;
                                               });
        }
        
        template <typename DocumentPredicate>
        vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate predicate) const {
            vector<Document> matched_documents = FindAllDocuments(ParseQuery(raw_query), predicate);
            
            sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
                     return lhs.rating > rhs.rating;
                 }
                 return lhs.relevance > rhs.relevance;
            });
            
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
        }
        
    private:
        struct DocumentData {
            int rating;
            DocumentStatus status;
        };
        
        const set<string> stop_words_;
        map<string, map<int, double>> word_to_document_freqs_;
        map<int, DocumentData> documents_;
        vector<int> document_ids_;
        
        static int ComputeAverageRating(const vector<int>& ratings);
        double ComputeWordInverseDocumentFreq(const string& word) const;
        vector<string> SplitIntoWordsNoStop(const string& text) const;
        
        //TODO выделить в class Query : public SearchServer
        struct QueryWord {
            string data;
            bool is_minus;
            bool is_stop;
            bool is_valid;
        };
        
        QueryWord ParseQueryWord(string text) const {
            bool is_minus = false, is_valid = true;
            if (text[0] == '-') {
                is_minus = true;
                text = text.substr(1);
            }
            
            //если после отрубания минуса осталась пустота или ещё один минус или слово содержит спецсимволы
            if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
                throw invalid_argument("query contains illegal characters"s);
            }
            
            return {text, is_minus, IsStopWord(text), is_valid};
        }
        
        struct Query {
            set<string> plus_words;
            set<string> minus_words;
        };
        
        Query ParseQuery(const string& text) const {
            Query query;
            for (const string& word : SplitIntoWords(text)) {
                const QueryWord query_word = ParseQueryWord(word);
                if (!query_word.is_stop) {
                    if (query_word.is_minus) {
                        query.minus_words.insert(query_word.data);
                    } else {
                        query.plus_words.insert(query_word.data);
                    }
                }
            }
            return query;
        }
        
        template <typename DocumentPredicate>
        vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicate) const {
            map<int, double> document_to_relevance;
            for (const string& word : query.plus_words) {
                if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }
                
                const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                    const auto& document_data = documents_.at(document_id);
                    if (predicate(document_id, document_data.status, document_data.rating)) {
                        document_to_relevance[document_id] += term_freq * inverse_document_freq;
                    }
                }
            }
            
            for (const string& word : query.minus_words) {
                if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }
                for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(document_id);
                }
            }
            
            vector<Document> matched_documents;
            for (const auto& [document_id, relevance] : document_to_relevance) {
                matched_documents.emplace_back(document_id, relevance, documents_.at(document_id).rating);
            }
            return matched_documents;
        }
    };
} //namespace std
