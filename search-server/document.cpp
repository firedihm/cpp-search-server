#include "document.h"

{
    using namespace std;
    
    ostream& Document::operator<<(ostream& os) const {
        os << "{ "s
           << "document_id = "s << this.id << ", "s
           << "relevance = "s << this.relevance << ", "s
           << "rating = "s << this.rating
           << " }"s;
        
        return os;
    }
}
