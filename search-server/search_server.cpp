#include "search_server.h"

#include <cmath>
#include <numeric>

using namespace std::string_literals;

const std::set<std::string>& SearchServer::GetStopWords() const {
    return stop_words_;
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(uint n) const {
    return document_ids_.at(n);
}

bool SearchServer::IsValidWord(const std::string& word) {
    return std::none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) != 0;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    if (document_id < 0 || documents_.count(document_id) != 0) {
        throw std::invalid_argument("document id "s + std::to_string(document_id) + " is invalid or already exists"s);
    }
    
    std::vector<std::string> words = ParseDocument(document);
    //if (words.empty()) {
    //    throw std::invalid_argument("document id "s + std::to_string(document_id) + " has no valid plus words"s);
    //}
    
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
}

std::vector<std::string> SearchServer::ParseDocument(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("invalid word "s + word + " was passed to document"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus sought_status) const {
    return FindTopDocuments(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                           return status == sought_status;
                                       });
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    Query query = ParseQuery(raw_query);
    
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
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

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    
    //если после отрубания минуса осталась пустота или ещё один минус или слово содержит спецсимволы
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("invalid word "s + text + " was passed to query"s);
    }
    
    return {text, is_minus, IsStopWord(text)};
}
