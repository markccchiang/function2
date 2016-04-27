
//  Copyright 2015-2016 Denis Blank <denis.blank at outlook dot com>
//     Distributed under the Boost Software License, Version 1.0
//       (See accompanying file LICENSE_1_0.txt or copy at
//             http://www.boost.org/LICENSE_1_0.txt)


#include "function2/function2.hpp"

using namespace fu2::detail;

namespace experimental {

struct invocation_wrapper_none {
  template<typename T>
  static T wrap(T&& functor) {
    return std::forward<T>(functor);
  }
};

struct invocation_wrapper_assert {
  template<typename T>
  static T wrap(T&&) {
    static_assert(always_false_t<T>::value, "This functor isn't accepted!")
  }
};

struct invocation_wrapper_method_this_ptr {
  template<typename T>
  static T wrap(T&& functor) {
    return std::forward<T>(functor);
  }
};

struct invocation_wrapper_method_this_ref {
  template<typename T>
  static T wrap(T&& functor) {
    return std::forward<T>(functor);
  }
};

template<bool Accept, typename InvocationWrapper = invocation_wrapper_none>
struct invocation_acceptor
  : std::integral_constant<bool, Accept>,
    InvocationWrapper { };

template<>
struct invocation_acceptor<false>
  : std::integral_constant<bool, false>,
    invocation_wrapper_assert { };

template<typename T, typename Signature, typename = always_void_t<>>
struct is_accepted_with
  : invocation_acceptor<false> { };

// Invocation acceptor which accepts (templated) functors and function pointers
template<typename T, typename ReturnType, typename... Args>
struct is_accepted_with<T, ReturnType(Args...),
  always_void_t<
    typename std::enable_if<std::is_convertible<
      decltype(std::declval<T>()(std::declval<Args>()...)),
      ReturnType
    >::value>::type>>
  : invocation_acceptor<true> { };

// Invocation acceptor which accepts (templated) class method pointers
// from pointer this
template<typename T, typename ReturnType, typename FirstArg, typename... Args>
struct is_accepted_with<T, ReturnType(FirstArg, Args...),
  always_void_t<
    typename std::enable_if<std::is_convertible<
      decltype((std::declval<FirstArg>()->*std::declval<T>())
                 (std::declval<Args>()...)),
      ReturnType
    >::value>::type>>
  : invocation_acceptor<true, invocation_wrapper_method_this_ptr> { };
}

using namespace experimental;

struct MyClass
{
  bool get(bool)
  {
    return true;
  }
};

int main(int, char**) {

  using tt = decltype(&MyClass::get);

  MyClass c;

  auto cc = &c;
  auto ptr = (&MyClass::get);

  (cc->*ptr)(true);

  using type = is_accepted_with<tt, bool(MyClass*, bool)>;

  invocation_wrapper_method_this_ptr m = type{};

  return 0;
}
