#pragma once

#include "document.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

/*
    большой бардак
    TODO найти способ вынести реализации шаблонов
*/

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words) : stop_words_(ParseStopWords(stop_words)) {}
    explicit SearchServer(const std::string &stop_words) : SearchServer(SplitIntoWords(stop_words)) {}
    
    const std::set<std::string>& GetStopWords() const;
    size_t GetDocumentCount() const;
    int GetDocumentId(uint n) const;
    
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;
    
    //вызов без явно переданного статуса или предиката = вызов с предикатом по умолчанию
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus sought_status = DocumentStatus::ACTUAL) const {
        return FindTopDocuments(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                               return status == sought_status;
                                           });
    }
    
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate predicate) const {
        std::vector<Document> matched_documents = FindAllDocuments(ParseQuery(raw_query), predicate);
        
        std::sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
             if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
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
    
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;
    
    static bool IsValidWord(const std::string& word);
    bool IsStopWord(const std::string& word) const;
    
    static int ComputeAverageRating(const std::vector<int>& ratings);
    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    
    template <typename StringContainer>
    std::set<std::string> ParseStopWords(const StringContainer& strings) {
        std::set<std::string> non_empty_strings;
        for (const std::string& str : strings) {
            if (!str.empty()) {
                if (!IsValidWord(str)) {
                    using namespace std::string_literals;
                    throw std::invalid_argument("invalid word "s + str + " was passed as stop word"s);
                }
                non_empty_strings.insert(str);
            }
        }
        return non_empty_strings;
    }
    std::vector<std::string> ParseDocument(const std::string& text) const;
    
    //TODO выделить в class Query : public SearchServer
    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(std::string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        
        //если после отрубания минуса осталась пустота или ещё один минус или слово содержит спецсимволы
        if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
            using namespace std::string_literals;
            throw std::invalid_argument("invalid word "s + text + " was passed to query"s);
        }
        
        return {text, is_minus, IsStopWord(text)};
    }
    
    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };
    
    Query ParseQuery(const std::string& text) const {
        Query query;
        for (const std::string& word : SplitIntoWords(text)) {
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
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicate) const {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.plus_words) {
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
        
        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
        
        std::vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance) {
            matched_documents.emplace_back(document_id, relevance, documents_.at(document_id).rating);
        }
        return matched_documents;
    }
};
