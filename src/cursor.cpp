#include "cursor.hpp"

#include "bplus_tree/bplus_tree.hpp"
#include "row/schema.hpp"
#include "row/row.hpp"
#include "varchar.hpp"
#include "row/row_view.hpp"
#include "exceptions.hpp"

namespace minisql {

/* Open the Cursor on bp_tree to read slots using schema.
 * Positions the Cursor to advance to the slot in bp_tree with key = origin. */
void Cursor::open(
    BPlusTree* bp_tree, const Schema* schema, const Row::Field& origin
) {
    bp_tree_ = bp_tree;
    schema_ = std::make_shared<Schema>(*schema);
    origin_ = origin;
    eof_ = false;
    leaf_node_ = nullptr;
    switch (schema_->primary().type) {
        case Schema::FieldType::INT:
            seek_ = &Cursor::seek__<int>;
            insert_ = &Cursor::insert__<int>;
            update_ = &Cursor::update__<int>;
            erase_ = &Cursor::erase__<int>;
            break;
        case Schema::FieldType::REAL:
            seek_ = &Cursor::seek__<double>;
            insert_ = &Cursor::insert__<double>;
            update_ = &Cursor::update__<double>;
            erase_ = &Cursor::erase__<double>;
            break;
        case Schema::FieldType::TEXT:
            seek_ = &Cursor::seek__<Varchar>;
            insert_ = &Cursor::insert__<Varchar>;
            update_ = &Cursor::update__<Varchar>;
            erase_ = &Cursor::erase__<Varchar>;
            break;
    }
}

/* Advance to the next slot.
 * Returns false if currently positioned on the last slot in bp_tree. */
bool Cursor::next() {
    if (eof_) return false;
    if (!leaf_node_) (this->*seek_)(origin_);
    else ++slot_;
    validate();
    return !eof_;
}

/* Return the current slot as a RowView.
 * Validates the Cursor before reading from the current slot.*/
RowView Cursor::current() {
    validate();
    if (eof_) throw DBConstraintViolation("Select failed: at eof.");
    return RowView{leaf_node_->slot(slot_), schema_};
}


// Remove access to any B+ Tree.
void Cursor::close() {
    bp_tree_ = nullptr;
    schema_ = nullptr;
    origin_ = 0;
    eof_ = true;
    if (leaf_node_) leaf_node_.reset();
}

/* Validate the current position of the Cursor.
 * If currently positioned beyond the end of leaf_node_ will attempt to move to
 * slot 0 of the next leaf.
 * Sets eof_ if positioned beyond the end of bp_tree. */
void Cursor::validate() {
    if (slot_ != leaf_node_->size()) return;
    if (!leaf_node_->is_rightmost()) {
        leaf_node_ = bp_tree_->open_leaf(leaf_node_->next_leaf());
        slot_ = 0;
    }
    else eof_ = true;
}

} // namespace minisql