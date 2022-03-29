#ifndef sand_node_hh
#define sand_node_hh

// A node is a processing level (akin to an event).  It is a node
// because the processing model supports a tree of nodes for
// processing.  A better name is probably appropriate.

#include <cstddef>

namespace sand {
  class node {
  public:
    explicit node(std::size_t id) : id_{id} {}
    std::size_t
    id() const noexcept
    {
      return id_;
    }

  private:
    std::size_t id_;
  };
}

#endif