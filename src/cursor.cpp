#include "cursor.hpp"

#include <memory>

#include "bplus_tree/bplus_tree.hpp"
#include "row/schema.hpp"
#include "field/field.hpp"
#include "field/varchar.hpp"
#include "row/row_view.hpp"
#include "exceptions.hpp"

namespace minisql {

// Intitialise the Cursor to read slots from bp_tree using schema.
Cursor::Cursor(BPlusTree* bp_tree, const Schema* schema)
    : bp_tree_{bp_tree}, schema_{std::make_shared<Schema>(*schema)} {
    switch (schema_->primary().type) {
        case FieldType::INT:
            seek_ = &Cursor::seek__<int>;
            insert_ = &Cursor::insert__<int>;
            erase_ = &Cursor::erase__<int>;
            break;
        case FieldType::REAL:
            seek_ = &Cursor::seek__<double>;
            insert_ = &Cursor::insert__<double>;
            erase_ = &Cursor::erase__<double>;
            break;
        case FieldType::TEXT:
            seek_ = &Cursor::seek__<Varchar>;
            insert_ = &Cursor::insert__<Varchar>;
            erase_ = &Cursor::erase__<Varchar>;
            break;
    }
}

// Position the Cursor to advance to the slot in bp_tree with key = origin.
void Cursor::open(const Field& origin) {
    origin_ = origin;
    eof_ = false;
    leaf_node_ = nullptr;
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


// Remove access to bp_tree_ and close leaf_node_.
void Cursor::close() {
    bp_tree_ = nullptr;
    schema_ = nullptr;
    origin_ = 0;
    eof_ = true;
    if (leaf_node_) leaf_node_.reset();
}

/* Validate the current position of the Cursor.
 * Attempts to move to slot 0 of the next leaf if positioned beyond the end of
 * leaf_node_ .
 * Sets eof_ if positioned beyond the end of bp_tree_. */
void Cursor::validate() {
    if (slot_ != leaf_node_->size()) return;
    if (!leaf_node_->is_rightmost()) {
        leaf_node_ = bp_tree_->open_leaf(leaf_node_->next_leaf());
        slot_ = 0;
    }
    else eof_ = true;
}

} // namespace minisql