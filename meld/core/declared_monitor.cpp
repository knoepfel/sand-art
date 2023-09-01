#include "meld/core/declared_monitor.hpp"

namespace meld {
  declared_monitor::declared_monitor(std::string name, std::vector<std::string> preceding_filters) :
    products_consumer{std::move(name), std::move(preceding_filters)}
  {
  }

  declared_monitor::~declared_monitor() = default;
}
