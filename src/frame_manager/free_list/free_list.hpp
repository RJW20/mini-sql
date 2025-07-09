#ifndef MINISQL_FREE_LIST_HPP
#define MINISQL_FREE_LIST_HPP

#include "frame_manager/cache/cache.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

/* Free List
 * A stack of free page_id_t's. */
class FreeList {
public:
    FreeList(Cache& cache, page_id_t first_free_list_block)
        : cache_{cache}, first_free_list_block_{first_free_list_block} {}

    page_id_t pop_back();
    void push_back(page_id_t pid);

    bool empty() const noexcept { return first_free_list_block_ == nullpid; }

    page_id_t first_free_list_block() const noexcept {
        return first_free_list_block_;
    }

private:
    Cache& cache_;
    page_id_t first_free_list_block_;
};

} // namespacde minisql

#endif // MINISQL_FREE_LIST_HPP