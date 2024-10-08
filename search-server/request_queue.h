#pragma once

#include "document.h"
#include "search_server.h"

#include <queue>

const static size_t MIN_IN_DAY = 1440;

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server)  : search_server_(search_server), no_result_requests_(0) {}
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate predicate);
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus sought_status = DocumentStatus::ACTUAL);

    uint GetNoResultRequests() const { return no_result_requests_; }
    
private:
    struct QueryResult {
        QueryResult(const std::string& query, int documents_found) : query(query), is_empty(!documents_found) {}
        
        bool empty() { return is_empty; }

        std::string query;
        bool is_empty;
    };
    
    const SearchServer& search_server_;
    std::queue<QueryResult> requests_;
    uint no_result_requests_;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate predicate) {
    std::vector<Document> result = search_server_.FindTopDocuments(raw_query, predicate);
    
    //избавляемся от записей, старше суток
    if (requests_.size() == MIN_IN_DAY) {
        if (requests_.front().empty()) {
            --no_result_requests_;
        }
        requests_.pop();
    }
    
    requests_.emplace(raw_query, result.size());
    
    if (result.empty()) {
        ++no_result_requests_;
    }
    
    return result;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus sought_status) {
    return AddFindRequest(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                              return status == sought_status;
                                     });
}
