#include "meld/core/declared_splitter.hpp"
#include "meld/model/handle.hpp"
#include "meld/model/level_counter.hpp"

namespace meld {

  generator::generator(product_store_const_ptr const& parent,
                       std::string const& node_name,
                       std::string const& new_level_name) :
    parent_{std::const_pointer_cast<product_store>(parent)},
    node_name_{node_name},
    new_level_name_{new_level_name}
  {
  }

  product_store_const_ptr generator::make_child(std::size_t const i, products new_products)
  {
    ++child_counts_[new_level_name_];
    return parent_->make_child(i, new_level_name_, node_name_, std::move(new_products));
  }

  product_store_const_ptr generator::flush_store() const
  {
    auto result = parent_->make_flush();
    if (not child_counts_.empty()) {
      result->add_product("[flush]",
                          std::make_shared<flush_counts const>(parent_->id()->level_name(),
                                                               std::move(child_counts_)));
    }
    return result;
  }

  declared_splitter::declared_splitter(std::string name,
                                       std::vector<std::string> preceding_filters) :
    products_consumer{std::move(name), std::move(preceding_filters)}
  {
  }

  declared_splitter::~declared_splitter() = default;
}
