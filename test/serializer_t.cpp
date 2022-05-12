#include "meld/graph/serial_node.hpp"
#include "meld/utilities/debug.hpp"
#include "meld/utilities/thread_counter.hpp"

#include "catch2/catch.hpp"
#include "oneapi/tbb/flow_graph.h"

using namespace meld;
using namespace oneapi::tbb;

TEST_CASE("Serialize functions based on resource", "[multithreading]")
{
  flow::graph g;
  unsigned int i{};
  flow::input_node src{g, [&i](flow_control& fc) {
                         if (i < 10) {
                           return ++i;
                         }
                         fc.stop();
                         return 0u;
                       }};

  serializer_node root_serializer{g};
  serializer_node genie_serializer{g};

  std::atomic<unsigned int> counter{};
  serial_node<unsigned int, 1> node1{g,
                                     [&counter](unsigned int const i) {
                                       thread_counter c{counter};
                                       debug("Processing from node 1 ", i);
                                       return i;
                                     },
                                     root_serializer};

  serial_node<unsigned int, 2> node2{g,
                                     [&counter](unsigned int const i) {
                                       thread_counter c{counter};
                                       debug("Processing from node 2 ", i);
                                       return i;
                                     },
                                     root_serializer,
                                     genie_serializer};

  serial_node<unsigned int, 1> node3{g,
                                     [&counter](unsigned int const i) {
                                       thread_counter c{counter};
                                       debug("Processing from node 3 ", i);
                                       return i;
                                     },
                                     genie_serializer};

  flow::function_node<unsigned int, unsigned int> receiving_node_1{
    g, flow::unlimited, [](unsigned int const i) {
      debug("Processed ROOT task ", i);
      return i;
    }};

  flow::function_node<unsigned int, unsigned int> receiving_node_2{
    g, flow::unlimited, [](unsigned int const i) {
      debug("Processed ROOT/GENIE task ", i);
      return i;
    }};

  flow::function_node<unsigned int, unsigned int> receiving_node_3{
    g, flow::unlimited, [](unsigned int const i) {
      debug("Processed GENIE task ", i);
      return i;
    }};

  make_edge(src, node1);
  make_edge(src, node2);
  make_edge(src, node3);
  make_edge(node1, receiving_node_1);
  make_edge(node2, receiving_node_2);
  make_edge(node3, receiving_node_3);

  src.activate();
  g.wait_for_all();
}
