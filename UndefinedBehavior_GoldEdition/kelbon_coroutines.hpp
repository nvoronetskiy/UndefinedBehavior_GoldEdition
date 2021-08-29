

#ifndef KELBON_COROUTINES_HPP
#define KELBON_COROUTINES_HPP

#include <coroutine>

#include "kelbon_concepts_functional.hpp"

namespace kelbon::coroutine {

    template<typename T>
    concept awaitable = requires(T) {
        &T::await_ready; // bool
        &T::await_suspend; // контролирует кому передать управление. - void - сразу вызывающему
        // bool - либо вызывающему/возобновляющему (true), либо обратно корутине(false)
        // other corutine handle - у другой корутины вызывается resume
        &T::await_resume; // добавление возвращающего значения позволяет co_await возвращать тип
    };

    template<typename T>
    concept awaiter = callable<T> && awaitable<func::result_type<T>>;


    template<func::returns<bool> auto ReadyChecker, callable auto Suspender, callable auto Resumer>
    struct default_awaitable {
    public:
        static constexpr bool is_noexcept_readycheck = signature<decltype(ReadyChecker)>::is_noexcept;
        static constexpr bool is_noexcept_resume = signature<decltype(Resumer)>::is_noexcept;
        static constexpr bool is_noexcept_suspender = signature<decltype(Suspender)>::is_noexcept;

        using resumer_t = decltype(Resumer);
        using suspender_t = decltype(Suspender);
        using checker_t = decltype(ReadyChecker);
        using other_handle_t = typename signature<suspender_t>::parameter_list::template get_element<0>;
        using resumer_arg_t = typename signature<resumer_t>::parameter_list::template get_element<0>;
    public:
        constexpr bool await_ready() noexcept(is_noexcept_readycheck) {
            return ReadyChecker();
        }
        constexpr func::result_type<suspender_t> await_suspend(other_handle_t handle)
            noexcept(is_noexcept_suspender)
            requires(!std::same_as<::kelbon::false_type, other_handle_t>) {
            return Suspender(handle);
        }
        constexpr func::result_type<suspender_t> await_suspend()
            noexcept(is_noexcept_suspender)
            requires(std::same_as<::kelbon::false_type, other_handle_t>) {
            return Suspender();
        }

        constexpr func::result_type<resumer_t> await_resume(resumer_arg_t arg)
            noexcept(is_noexcept_resume)
            requires(!std::same_as<::kelbon::false_type, other_handle_t>) {
            return Resumer(arg);
        }
        constexpr func::result_type<resumer_t> await_resume()
            noexcept(is_noexcept_resume)
            requires(std::same_as<::kelbon::false_type, other_handle_t>) {
            return Resumer();
        }
    };

    template<bool IsImmediateStart, bool IsResumable = true, typename ResultType = void, typename Tranformation = void>//TODO трансформация это нечто вида void(auto x)
    struct promise_type_template {
        using co_handle = std::coroutine_handle<promise_type_template>;
        using result_type = ResultType;
        static constexpr bool resumable = IsResumable;
        result_type storage;

        [[nodiscard]] auto get_return_object() noexcept {
            return co_handle::from_promise(*this);
        }
        auto initial_suspend() const {
            return std::suspend_never{};
        }
        auto final_suspend() const noexcept {
            return std::suspend_always{};
        }
        //void return_void() const // видимо должно быть что то одно, либо return void, либо return value
        //{}
        void return_value(result_type&& value) {
            storage = std::forward<result_type>(value);
        }
        template<typename..., typename FakeArg = ResultType>
        requires(!std::is_void_v<FakeArg>)
        auto yield_value(FakeArg&& value) noexcept(std::is_nothrow_copy_constructible_v<FakeArg>) {
            storage = std::forward<result_type>(value);
            return std::suspend_always{};
        }
        [[noreturn]] void unhandled_exception() {
            throw;
        }
       

        //template<typename T> // accept any type , but CE if await transform cant transform it
        //void await_transform(T&& value) // additional option = delete; what the hell omg
        //{ must return awaitable object
        // transformer(std::forward<T>(value));
        // }
       
       // template<typename T> //requires ... если такого метода нет(requires false), то никаких преобразований, иначе вызов при await <expr>
      //  auto await_transform(T&& value) {
      //      return std::suspend_always{};
      //  }
    };

    template<typename PromiseType>
    struct coroutine {
        using promise_type = PromiseType;
        using co_handle = typename promise_type::co_handle;
        using promise_result_type = typename promise_type::result_type;
    private:
        co_handle my_handle;
    public:
        constexpr coroutine() noexcept = default;
        constexpr coroutine(co_handle handle)
            noexcept(std::is_nothrow_copy_constructible_v<co_handle>)
            : my_handle(handle)
        {}
        ~coroutine() {
            if (my_handle) {
                my_handle.destroy();
            }
        }

        promise_result_type Get() const requires(!std::is_void_v<promise_result_type>) {
            resume();
            return my_handle.promise().storage;
        }
        std::suspend_always resume() const {// requires(promise_type::is_resumable) {
            my_handle.resume();
            return {};
        }
    };

    template<typename Result>
    using generator = coroutine<promise_type_template<false, true, Result>>;

} // namespace kelbon::coroutine

#endif // !KELBON_COROUTINES_HPP
