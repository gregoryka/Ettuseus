
#include <concepts>
#include <coroutine>
#include <exception>

template <typename T> struct Generator {
  // The class name 'Generator' is our choice and it is not required for
  // coroutine magic. Compiler recognizes coroutine by the presence of
  // 'co_yield' keyword. You can use name 'MyGenerator' (or any other name)
  // instead as long as you include nested struct promise_type with 'MyGenerator
  // get_return_object()' method. (Note: It is necessary to adjust the
  // declarations of constructors and destructors
  //  when renaming.)

  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type // required
  {
    T value_;
    std::exception_ptr exception_;

    auto get_return_object() -> Generator {
      return {handle_type::from_promise(*this)};
    }
    auto initial_suspend() -> std::suspend_always { return {}; }
    auto final_suspend() noexcept -> std::suspend_always { return {}; }
    void unhandled_exception() {
      exception_ = std::current_exception();
    } // saving
      // exception

    template <std::convertible_to<T> From> // C++20 concept
    auto yield_value(From &&from) -> std::suspend_always {
      value_ = std::forward<From>(from); // caching the result in promise
      return {};
    }
    void return_void() {}
  };

  handle_type handle_;

  Generator(handle_type handle) : handle_(handle) {}
  Generator(const Generator &) = delete;
  ~Generator() { handle_.destroy(); }
  explicit operator bool() {
    fill(); // The only way to reliably find out whether or not we finished
            // coroutine, whether or not there is going to be a next value
            // generated (co_yield) in coroutine via C++ getter (operator ()
            // below) is to execute/resume coroutine until the next co_yield
            // point (or let it fall off end). Then we store/cache result in
            // promise to allow getter (operator() below to grab it without
            // executing coroutine).
    return !handle_.done();
  }
  auto operator()() -> T {
    fill();
    full_ = false; // we are going to move out previously cached
                   // result to make promise empty again
    return std::move(handle_.promise().value_);
  }

private:
  bool full_ = false;

  void fill() {
    if (!full_) {
      handle_();
      if (handle_.promise().exception_) {
        std::rethrow_exception(handle_.promise().exception_);
      }
      // propagate coroutine exception in called context

      full_ = true;
    }
  }
};
