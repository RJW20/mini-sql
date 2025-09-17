#ifndef MINISQL_CURSOR_HPP
#define MINISQL_CURSOR_HPP

#include <memory>

#include "bplus_tree/bplus_tree.hpp"
#include "row/schema.hpp"
#include "field.hpp"
#include "row/row_view.hpp"
#include "bplus_tree/leaf_node.hpp"
#include "bplus_tree/node.hpp"
#include "exceptions.hpp"

namespace minisql {

/* Cursor
 * An interface for traversing and reading Rows from the LeafNodes of a B+
 * Tree. */
class Cursor {
public:
    Cursor(BPlusTree* bp_tree, const Schema* schema);

    void open(const Field& origin = 0);
    void seek(const Field& key) { (this->*seek_)(key); }
    bool next();
    RowView current();
    void insert(const RowView& rv) { (this->*insert_)(rv); }
    void erase() { (this->*erase_)(); }

    void close();

private:
    BPlusTree* bp_tree_;
    std::shared_ptr<Schema> schema_;
    Field origin_ {0};
    bool eof_ {true};
    std::unique_ptr<LeafNode> leaf_node_ {nullptr};
    Node::size_t slot_;

    void (Cursor::* seek_)(const Field&);
    void (Cursor::* insert_)(const RowView&);
    void (Cursor::* erase_)();

    void validate();

    template <typename Key>
    void seek__(const Field& key) {
        Key key_ = std::get<Key>(key);
        leaf_node_ = bp_tree_->seek_leaf<Key>(key_);
        slot_ = BPlusTree::seek_slot<Key>(leaf_node_.get(), key_);
    }

    template <typename Key>
    void insert__(const RowView& rv) {
        if (slot_ < leaf_node_->size() && 
            leaf_node_->key<Key>(slot_) == std::get<Key>(rv.primary()))
            throw DBConstraintViolation(
                "Insert failed: primary key already exists."
            );
        bp_tree_->insert_into<Key>(leaf_node_.get(), slot_, rv.data());
        if (eof_) eof_ = false;
    }

    template <typename Key>
    void erase__() {
        validate();
        if (eof_) throw DBConstraintViolation("Delete failed: at eof.");
        if (slot_ + 1 != leaf_node_->size()) {
            Key next_key = leaf_node_->key<Key>(slot_ + 1);
            bp_tree_->erase_from<Key>(leaf_node_.get(), slot_);
            seek__<Key>(next_key);
            return;
        }
        if (!leaf_node_->is_rightmost()) {
            std::unique_ptr<LeafNode> next_leaf = bp_tree_->open_leaf(
                leaf_node_->next_leaf()
            );
            Key next_key = next_leaf->key<Key>(0);
            bp_tree_->erase_from<Key>(leaf_node_.get(), slot_);
            seek__<Key>(next_key);
            return;
        }
        bp_tree_->erase_from<Key>(leaf_node_.get(), slot_);
        eof_ = true;
    }
};

} // namespace minisql

#endif // MINISQL_CURSOR_HPP