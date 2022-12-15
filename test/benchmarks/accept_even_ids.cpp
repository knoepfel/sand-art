#include "meld/graph/transition.hpp"
#include "meld/module.hpp"

#include <string>

using namespace meld::concurrency;

namespace {
  bool accept_even_ids(meld::level_id const& id) { return id.back() % 2 == 0; }
}

DEFINE_MODULE(m, config)
{
  m.declare_filter("accept_even_ids", accept_even_ids)
    .concurrency(unlimited)
    .input(value_to<std::string>(config.at("product_name")));
}
