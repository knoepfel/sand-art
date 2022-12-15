#include "meld/module.hpp"

#include <algorithm>
#include <string>

using namespace meld::concurrency;

namespace {
  auto fibs_less_than(int const n)
  {
    std::vector<int> result;
    int i = 0;
    int j = 1;
    int sum = 0;
    while (sum < n) {
      result.push_back(sum);
      sum = i + j;
      i = j;
      j = sum;
    }
    return result;
  }

  class fibonacci_numbers {
  public:
    explicit fibonacci_numbers(int const n) : numbers_{fibs_less_than(n + 1)} {}
    bool accept(int i) const { return std::binary_search(begin(numbers_), end(numbers_), i); }

  private:
    std::vector<int> numbers_;
  };
}

DEFINE_MODULE(m, config)
{
  m.make<fibonacci_numbers>(config.at("max_number").as_int64())
    .declare_filter("accept_fibonacci_numbers", &fibonacci_numbers::accept)
    .concurrency(unlimited)
    .input(value_to<std::string>(config.at("product_name")));
}
