#include "engine/database_handle.hpp"

#include "engine/engine.hpp"

namespace minisql {

DatabaseHandle::~DatabaseHandle() { engine_.release_database(path_); }

} // namespace minisql