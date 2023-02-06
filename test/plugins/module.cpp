#include "meld/module.hpp"
#include "test/plugins/add.hpp"

using namespace meld;
using namespace meld::concurrency;

// TODO: Option to select which algorithm to run via configuration?

DEFINE_MODULE(m)
{
  m.declare_transform(test::add).concurrency(unlimited).react_to("i", "j").output("sum");
  m.declare_monitor(test::verify).concurrency(unlimited).input(react_to("sum"), use(0));
}