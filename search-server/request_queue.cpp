#include "request_queue.h"

namespace std {
    uint RequestQueue::GetNoResultRequests() const {
        return no_result_requests_;
    }
} //namespace std
