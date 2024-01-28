#include "search_server.h"

#include <numeric>

const std::set<std::string>& SearchServer::GetStopWords() const {
    return stop_words_;
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(uint n) const {
    if (n > document_ids_.size()) {
        return -1;
    }
    return document_ids_.at(n);
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
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

bool SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    using namespace std::string_literals;
    
    if (document_id < 0) {
        std::cerr << "couldn't add document "s << document_id << ": negative id"s << std::endl;
        return false;
    }
    
    if (documents_.count(document_id) != 0) {
        std::cerr << "couldn't add document "s << document_id << ": id already exists"s << std::endl;
        return false;
    }
    
    std::vector<std::string> words = SplitIntoWordsNoStop(document);
    if (words.empty()) {
        std::cerr << "couldn't add document "s << document_id << ": document has no valid plus words"s << std::endl;
        return false;
    }
    
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
    return true;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (IsValidWord(word) && !IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
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
