#ifndef meld_core_store_counters_hpp
#define meld_core_store_counters_hpp

#include "meld/core/fwd.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"

#include "oneapi/tbb/concurrent_hash_map.h"

#include <atomic>

namespace meld {
  class store_flag {
  public:
    void flush_received(std::size_t original_message_id);
    bool is_flush(level_id const* id = nullptr) noexcept;
    void mark_as_processed() noexcept;
    unsigned int original_message_id() const noexcept;

  private:
    std::atomic<bool> flush_received_{false};
    std::atomic<bool> processed_{false};
    std::size_t original_message_id_{}; // Necessary for matching inputs to downstream join nodes.
  };

  class detect_flush_flag {
    using flags_t = tbb::concurrent_hash_map<level_id::hash_type, std::unique_ptr<store_flag>>;

  protected:
    using flag_accessor = flags_t::accessor;
    using const_flag_accessor = flags_t::const_accessor;
    store_flag& flag_for(level_id::hash_type hash, flag_accessor& ca);
    bool flag_for(level_id::hash_type hash, const_flag_accessor& ca) const;
    void erase_flag(const_flag_accessor& ca);

  private:
    flags_t flags_;
  };

  // =========================================================================

  class store_counter {
  public:
    void set_flush_value(product_store_const_ptr const& ptr, std::size_t original_message_id);
    void increment() noexcept;
    bool is_flush(level_id const* id = nullptr) noexcept;
    unsigned int original_message_id() const noexcept;

  private:
    std::atomic<unsigned int> count_{};
    std::atomic<unsigned int> stop_after_{-1u};
    unsigned int original_message_id_{}; // Necessary for matching inputs to downstream join nodes.
  };

  class count_stores {
    using counters_t =
      tbb::concurrent_hash_map<level_id::hash_type, std::unique_ptr<store_counter>>;

  protected:
    using counter_accessor = counters_t::accessor;
    using const_counter_accessor = counters_t::const_accessor;
    store_counter& counter_for(level_id::hash_type hash, counter_accessor& ca);
    bool counter_for(level_id::hash_type hash, const_counter_accessor& ca) const;
    void erase_counter(const_counter_accessor& ca);

  private:
    counters_t counters_;
  };
}

#endif /* meld_core_store_counters_hpp */