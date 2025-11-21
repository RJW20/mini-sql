#ifndef MINISQL_ENGINE_EXCEPTIONS_HPP
#define MINISQL_ENGINE_EXCEPTIONS_HPP

#include <cstddef>
#include <string>
#include <type_traits>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "minisql/exception.hpp"
#include "minisql/varchar.hpp"

namespace minisql {

// Base class for exceptions occurring during engine execution.
class EngineException : public Exception {
public:
    explicit EngineException(const std::string& msg)
        : Exception("Engine error: " + msg) {}
};

// Thrown when a byte_io function is not given enough bytes.
class ByteIOException : public EngineException {
public:
    ByteIOException(
        const std::string& action, std::size_t expected, std::size_t got
    ) : EngineException(
        "insufficent bytes for byte_io::" + action + " (expected " +
        std::to_string(expected) + " bytes, got " + std::to_string(got) +
        " bytes)"
    ) {}
};

// Thrown when the on-disk file size does not match expected layout.
class DiskException : public EngineException {
public:
    DiskException(std::size_t expected, std::size_t got)
        : EngineException(
            "file size mismatch (expected " + std::to_string(expected) +
            " bytes, got " + std::to_string(got) + " bytes)"
        ) {}
};

/* Base class for exceptions occurring when pinning pages to or unpinning pages
 * from frames. */
class CacheException : public EngineException {
public:
    using EngineException::EngineException;
};

// Thrown when a page cannot be unpinned.
class CacheUnpinException : public CacheException {
public:
    CacheUnpinException(page_id_t pid, const std::string& reason)
        : CacheException(
            "unable to unpin page " + std::to_string(pid) + " - " + reason
        ) {}
};

// Thrown when all pages in the cache are pinned.
class CacheCapacityException : public CacheException {
public:
    CacheCapacityException()
        : CacheException(
        "all pages in the cache are pinned - cannot evict") {}
};

// Thrown when a file or page header has an unexpected magic number.
class MagicException : public EngineException {
public:
    explicit MagicException(Magic magic)
        : EngineException(
            "invalid header format - bad magic number " +
            std::to_string(static_cast<std::underlying_type_t<Magic>>(magic))
        ) {}
};

// Thrown when two b+ tree nodes are incompatible (key or slot size mismatch).
class NodeIncompatibilityException : public EngineException {
public:
    using key_size_t = NodeHeader::key_size_t;
    using slot_size_t = NodeHeader::slot_size_t;

    NodeIncompatibilityException(
        key_size_t key_1, key_size_t key_2,
        slot_size_t slot_1, slot_size_t slot_2
    ) : EngineException(
        "incompatible B+ Tree nodes (key sizes " + std::to_string(key_1) +
        " vs " + std::to_string(key_2) + ", slot sizes " +
        std::to_string(slot_1) + " vs " + std::to_string(slot_2) + ")"
    ) {}
};

// Base class for exceptions occurring during cursor operations.
class CursorException : public EngineException {
public:
    using EngineException::EngineException;
};

// Thrown when a key already exists within a B+ Tree.
template <typename T>
class DuplicateKeyException : public CursorException {
    static_assert(std::is_arithmetic_v<T>);
public:
    explicit DuplicateKeyException(T key)
        : CursorException("key " + std::to_string(key) + " already exists") {}
};

template <>
class DuplicateKeyException<Varchar> : public CursorException {
public:
    explicit DuplicateKeyException(const Varchar& key)
        : CursorException(
            "key \"" + std::string(key.data()) + "\" already exists"
        ) {}
};

// Thrown when a cursor is position beyond the end of a tree.
class EndOfTreeException : public CursorException {
public:
    explicit EndOfTreeException(const std::string& action)
        : CursorException(action + " failed - at end of tree") {}
};

// Thrown when a numerical operation cannot be compiled.
class CompilationException : public EngineException {
public:
    explicit CompilationException(const std::string& operation)
        : EngineException(
            "operation \"" + operation + "\" can only be compiled for " + 
            "numerical types"
        ) {}
};

} // namespace minisql

#endif // MINISQL_ENGINE_EXCEPTIONS_HPP