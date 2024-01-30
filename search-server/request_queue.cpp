#include "request_queue.h"

uint RequestQueue::GetNoResultRequests() const {
    return no_result_requests_;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus sought_status) {
    return AddFindRequest(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                              return status == sought_status;
                                     });
}
