#include "meld/core/declared_filter.hpp"

namespace meld {
  declared_filter::declared_filter(std::string name, std::vector<std::string> preceding_filters) :
    products_consumer{std::move(name), std::move(preceding_filters)}
  {
  }

  declared_filter::~declared_filter() = default;
}
