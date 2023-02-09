// =======================================================================================
// This test executes the following graph
//
//            Multiplexer
//            /        \
//           /          \
//      job_add(*)     run_add(^)
//          |             |\
//          |             | \
//          |             |  \
//   verify_job_sum       |   \
//                        |    \
//             verify_run_sum   \
//                               \
//                           two_layer_job_add(*)
//                                |
//                                |
//                         verify_two_layer_job_sum
//
// where the asterisk (*) indicates a reduction step over the full job, and the caret (^)
// represents a reduction step over each run.
// =======================================================================================

#include "meld/core/cached_product_stores.hpp"
#include "meld/core/framework_graph.hpp"
#include "meld/model/level_hierarchy.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"

#include "catch2/catch.hpp"
#include "spdlog/spdlog.h"

#include <atomic>
#include <string>
#include <vector>

using namespace meld;
using namespace meld::concurrency;

namespace {
  struct counter {
    std::atomic<unsigned int> number;
    auto send() const { return number.load(); }
  };

  void add(counter& result, unsigned int number) { result.number += number; }
  void verify(unsigned int sum, unsigned expected) { CHECK(sum == expected); }
}

TEST_CASE("Different levels of reduction", "[graph]")
{
  constexpr auto index_limit = 2u;
  constexpr auto number_limit = 5u;
  std::vector<level_id_ptr> levels;
  levels.reserve(1 + index_limit * (number_limit + 1u));
  auto job_id = levels.emplace_back(level_id::base_ptr());
  for (unsigned i = 0u; i != index_limit; ++i) {
    auto run_id = levels.emplace_back(job_id->make_child(i, "run"));
    for (unsigned j = 0u; j != number_limit; ++j) {
      levels.push_back(run_id->make_child(j, "event"));
    }
  }

  auto it = cbegin(levels);
  auto const e = cend(levels);
  cached_product_stores cached_stores{};
  framework_graph g{[&cached_stores, it, e]() mutable -> product_store_ptr {
    if (it == e) {
      return nullptr;
    }
    auto const& id = *it++;

    auto store = cached_stores.get_store(id);
    if (id->level_name() == "event") {
      store->add_product<unsigned>("number", id->number());
    }
    return store;
  }};

  g.declare_reduction("run_add", add, 0)
    .concurrency(unlimited)
    .react_to("number")
    .output("run_sum")
    .over("run");
  g.declare_reduction("job_add", add, 0)
    .concurrency(unlimited)
    .react_to("number")
    .output("job_sum")
    .over("job");
  g.declare_reduction("two_layer_job_add", add, 0)
    .concurrency(unlimited)
    .react_to("run_sum")
    .output("two_layer_job_sum")
    .over("job");

  g.declare_monitor("verify_run_sum", verify)
    .concurrency(unlimited)
    .input(react_to("run_sum"), use(10u));
  g.declare_monitor("verify_two_layer_job_sum", verify)
    .concurrency(unlimited)
    .input(react_to("two_layer_job_sum"), use(20u));
  g.declare_monitor("verify_job_sum", verify)
    .concurrency(unlimited)
    .input(react_to("job_sum"), use(20u));

  g.execute();

  CHECK(g.execution_counts("run_add") == index_limit * number_limit);
  CHECK(g.execution_counts("job_add") == index_limit * number_limit);
  CHECK(g.execution_counts("two_layer_job_add") == index_limit);
  CHECK(g.execution_counts("verify_run_sum") == index_limit);
  CHECK(g.execution_counts("verify_two_layer_job_sum") == 1);
  CHECK(g.execution_counts("verify_job_sum") == 1);
}