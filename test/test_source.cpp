#include "sand/core/data_levels.hpp"
#include "sand/core/source.hpp"

#include <iostream>

namespace sand::test {
  class my_source {
  public:
    explicit my_source(std::size_t const n) : num_nodes_{n} {}

    std::shared_ptr<node>
    data()
    {
      if (cursor_ < num_nodes_) {
        ++cursor_;
        std::cout << "Creating run " << cursor_ << '\n';
        return std::make_shared<run>(cursor_);
      }
      return nullptr;
    }

  private:
    std::size_t num_nodes_;
    std::size_t cursor_{0};
  };

  SAND_REGISTER_SOURCE(my_source)
}