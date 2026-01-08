#ifndef MINISQL_INTERNAL_NODE_HPP
#define MINISQL_INTERNAL_NODE_HPP

#include <cstddef>
#include <utility>

#include "bplus_tree/node.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"

namespace minisql {

/* Internal Node
 * A speciailisation of Node in which slots are comprised of a key and a
 * page_id_t. */
class InternalNode : public Node {
public:
    InternalNode(
        FrameView&& fv, key_size_t key_size, page_id_t parent,
        page_id_t first_child = nullpid
    );
    InternalNode(FrameView&& fv) : Node{std::move(fv)} {}

    bool is_leaf() const override final { return false; }

    page_id_t child(size_t slot) const {
        if (slot == static_cast<size_t>(-1)) return first_child();
        return fv_.view<page_id_t>(offset(slot) + key_size_);
    }
    void set_child(size_t slot, page_id_t pid) {
        if (slot == static_cast<size_t>(-1)) set_first_child(pid);
        else fv_.write<page_id_t>(offset(slot) + key_size_, pid);
    }

    template <typename Key>
    void insert(size_t slot, const Key& key, page_id_t pid) {
        shift(slot, 1);
        set_key<Key>(slot, key);
        set_child(slot, pid);
    }

    template <typename Key>
    static Key split(InternalNode* dst, InternalNode* src);
    template <typename Key>
    static void merge(
        InternalNode* dst, InternalNode* src, const Key& separator
    );

    template <typename Key>
    static Key take_back(
        InternalNode* dst, InternalNode* src, const Key& separator
    );
    template <typename Key>
    static Key take_front(
        InternalNode* dst, InternalNode* src, const Key& separator
    );


private:
    page_id_t first_child() const {
        return fv_.view<page_id_t>(InternalNodeHeader::FIRST_CHILD_OFFSET);
    }
    void set_first_child(page_id_t pid) {
        fv_.write<page_id_t>(InternalNodeHeader::FIRST_CHILD_OFFSET, pid);
    }

    std::size_t header_size() const override final {
        return InternalNodeHeader::SIZE;
    }

    size_t min_size() const override final {
        if (is_root()) return 1;
        size_t max_size_ = max_size();
        return max_size_ / 2 + max_size_ % 2 - 1;
    }
};

// Extern declarations for explicitly instantiated template methods.
#define FIELD_TYPE(T)                                                         \
    extern template T InternalNode::split<T>(InternalNode*, InternalNode*);   \
    extern template void InternalNode::merge<T>(                              \
        InternalNode*, InternalNode*, const T&                                \
    );                                                                        \
    extern template T InternalNode::take_back<T>(                          \
        InternalNode*, InternalNode*, const T&                                \
    );                                                                        \
    extern template T InternalNode::take_front<T>(                         \
        InternalNode*, InternalNode*, const T&                                \
    );
#define FIELD_TYPE_LAST(T) FIELD_TYPE(T)

#include "minisql/field_types.def"
#undef FIELD_TYPE
#undef FIELD_TYPE_LAST

} // namespace minisql

#endif // MINISQL_INTERNAL_NODE_HPP