#ifndef meld_core_user_functions_hpp
#define meld_core_user_functions_hpp

#include "meld/core/declared_reduction.hpp"
#include "meld/core/declared_splitter.hpp"
#include "meld/core/declared_transform.hpp"

#include "oneapi/tbb/flow_graph.h"
#include "oneapi/tbb/global_control.h"

#include <cassert>
#include <concepts>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

namespace meld {

  // ==============================================================================
  // Registering user functions

  struct void_tag {};

  template <typename T>
  class user_functions {
  public:
    user_functions(tbb::flow::graph& g,
                   declared_transforms& transforms,
                   declared_reductions& reductions,
                   declared_splitters& splitters) requires(std::same_as<T, void_tag>) :
      graph_{g}, transforms_{transforms}, reductions_{reductions}, splitters_{splitters}
    {
    }
    template <typename... Args>
    user_functions(tbb::flow::graph& g,
                   declared_transforms& transforms,
                   declared_reductions& reductions,
                   declared_splitters& splitters,
                   Args&&... args) requires(not std::same_as<T, void_tag>) :
      graph_{g},
      transforms_{transforms},
      reductions_{reductions},
      splitters_{splitters},
      bound_obj_{std::make_unique<T>(std::forward<Args>(args)...)}
    {
    }

    // Transforms
    template <typename R, typename... Args>
    auto declare_transform(std::string name, R (*f)(Args...)) requires std::same_as<T, void_tag>
    {
      assert(not bound_obj_);
      return incomplete_transform{*this, name, graph_, f};
    }

    template <typename R, typename... Args>
    auto declare_transform(std::string name, R (T::*f)(Args...))
    {
      assert(bound_obj_);
      return incomplete_transform<T, R, Args...>{
        *this,
        name,
        graph_,
        [bound_obj = std::move(*bound_obj_), f](Args const&... args) mutable -> R {
          return (bound_obj.*f)(args...);
        }};
    }

    template <typename R, typename... Args>
    auto declare_transform(std::string name, R (T::*f)(Args...) const)
    {
      assert(bound_obj_);
      return incomplete_transform<T, R, Args...>{
        *this,
        name,
        graph_,
        [bound_obj = std::move(*bound_obj_), f](Args const&... args) mutable -> R {
          return (bound_obj.*f)(args...);
        }};
    }

    // Reductions
    template <typename R, typename... Args, typename... InitArgs>
    auto declare_reduction(std::string name,
                           void (*f)(R&, Args...),
                           InitArgs&&... init_args) requires std::same_as<T, void_tag>
    {
      assert(not bound_obj_);
      return incomplete_reduction{
        *this, name, graph_, f, std::make_tuple(std::forward<InitArgs>(init_args)...)};
    }

    template <typename R, typename... Args, typename... InitArgs>
    auto declare_reduction(std::string name, void (T::*f)(R&, Args...), InitArgs&... init_args)
    {
      assert(bound_obj_);
      return incomplete_reduction<T, R, Args...>{
        *this,
        name,
        graph_,
        [bound_obj = std::move(*bound_obj_), f](R& result, Args const&... args) mutable {
          return (bound_obj.*f)(result, args...);
        },
        std::make_tuple(std::forward<InitArgs>(init_args)...)};
    }

    template <typename R, typename... Args, typename... InitArgs>
    auto declare_reduction(std::string name,
                           void (T::*f)(R&, Args...) const,
                           InitArgs&&... init_args)
    {
      assert(bound_obj_);
      return incomplete_reduction<T, R, Args...>{
        *this,
        name,
        graph_,
        [bound_obj = std::move(*bound_obj_), f](R& result, Args const&... args) mutable {
          return (bound_obj.*f)(result, args...);
        },
        std::make_tuple(std::forward<InitArgs>(init_args)...)};
    }

    // Splitters
    template <typename... Args>
    auto declare_splitter(std::string name, void (*f)(Args...)) requires std::same_as<T, void_tag>
    {
      assert(not bound_obj_);
      return incomplete_splitter{*this, name, graph_, f};
    }

    // Expert-use only
    void add_transform(std::string const& name, declared_transform_ptr ptr)
    {
      transforms_.try_emplace(name, std::move(ptr));
    }

    void add_reduction(std::string const& name, declared_reduction_ptr ptr)
    {
      reductions_.try_emplace(name, std::move(ptr));
    }

    void add_splitter(std::string const& name, declared_splitter_ptr ptr)
    {
      splitters_.try_emplace(name, std::move(ptr));
    }

  private:
    tbb::flow::graph& graph_;
    declared_transforms& transforms_;
    declared_reductions& reductions_;
    declared_splitters& splitters_;
    std::unique_ptr<T> bound_obj_;
  };
}

#endif /* meld_core_user_functions_hpp */
