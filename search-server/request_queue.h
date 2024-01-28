#include "search_server.h"

#include <queue>

namespace std {
    const static size_t MIN_IN_DAY = 1440;
    
    class RequestQueue {
    public:
        explicit RequestQueue(const SearchServer& search_server)  : search_server_(search_server), no_result_requests_(0) {}
        
        uint GetNoResultRequests() const;
        
        //вызов без явно переданного статуса или предиката = вызов с предикатом по умолчанию
        vector<Document> AddFindRequest(const string& raw_query, DocumentStatus sought_status = DocumentStatus::ACTUAL) {
            return AddFindRequest(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                                      return status == sought_status;
                                                  });
        }
        
        template <typename DocumentPredicate>
        vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate predicate) {
            vector<Document> result = search_server_.FindTopDocuments(raw_query, predicate);
            
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
        
    private:
        struct QueryResult {
            string query;
            bool is_empty;
            
            QueryResult(const string& query, int documents_found) : query(query), is_empty(!documents_found) {}
            
            bool empty() { return is_empty; }
        };
        
        const SearchServer& search_server_;
        queue<QueryResult> requests_;
        uint no_result_requests_;
    };
} //namespace std
