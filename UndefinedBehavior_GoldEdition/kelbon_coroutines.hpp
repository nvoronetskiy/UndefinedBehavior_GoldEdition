

#ifndef KELBON_COROUTINES_HPP
#define KELBON_COROUTINES_HPP

#include <array>
#include <coroutine>
#include <iterator>
#include <tuple>

#include "kelbon_concepts_base.hpp"
#include "kelbon_concepts_functional.hpp"
#include "kelbon_type_traits_advanced.hpp"
#include "kelbon_utility.hpp"

namespace kelbon::coro {

    template<typename T>
    struct return_block : return_block<void> {
        using result_type = T;

        result_type storage;
        // ������ ������ �� return_value ��� ����� ����� ��� ������, 99% ��� ���� �������� �������� � ����� ������ ��������, ���� ��������� ��� � range_based loop,
        // ���� ����������� ��� ������������� �������� �������� ��� �������� � ����������
        constexpr auto yield_value(result_type value)
            noexcept(std::is_nothrow_move_assignable_v<result_type>) {
            storage = std::move(value);
            return std::suspend_always{};
        }
    };

    template<>
    struct return_block<void> {
        constexpr void return_void() const noexcept
        {}
    };

    template<bool IsImmediateStart, bool IsResumable = true, bool StopBeforeEnd = true, typename ResultType = void>
    struct base_promise_type
        : return_block<ResultType> {
    private:
        using ret_block = return_block<ResultType>;
    public:
        using co_handle = std::coroutine_handle<base_promise_type>;
        static constexpr bool is_resumable = IsResumable;

        using ret_block::ret_block;

        [[nodiscard]] auto get_return_object() noexcept {
            return co_handle::from_promise(*this);
        }
        constexpr auto initial_suspend() const noexcept {
            if constexpr (IsImmediateStart) {
                return std::suspend_never{};
            }
            else {
                return std::suspend_always();
            }
        }
        constexpr auto final_suspend() const noexcept {
            if constexpr (StopBeforeEnd) {
                return std::suspend_always{};
            }
            else {
                return std::suspend_never{};
            }
        }

        [[noreturn]] void unhandled_exception() {
            throw;
        }
    };

    template<typename PromiseType>
    struct coroutine {
        using promise_type = PromiseType;
        using co_handle = typename promise_type::co_handle;
    protected:
        co_handle my_handle;
    public:
        constexpr coroutine() noexcept = default;
        constexpr coroutine(co_handle handle)
            noexcept(std::is_nothrow_copy_constructible_v<co_handle>)
            : my_handle(handle)
        {}
        coroutine(coroutine&& another) noexcept
        : my_handle(another.my_handle) {
            another.my_handle = nullptr;
        }
        coroutine& operator=(coroutine&& another) noexcept {
            if (this == &another) {
                return;
            }
            if (my_handle) {
                my_handle.destroy();
            }
            my_handle = another.my_handle;
            another.my_handle = nullptr;
        }
        void Resume() const
            requires(promise_type::is_resumable) {
            my_handle.resume();
        }
        [[nodiscard]] constexpr bool Done() const noexcept {
            return my_handle.done();
        }
        ~coroutine() {
            if (my_handle) {
                my_handle.destroy();
            }
        }
    };

    template<typename ResultType>
    struct generator
        : coroutine<base_promise_type<false, true, true, ResultType>> {
    private:
        using base_t = coroutine<base_promise_type<false, true, true, ResultType>>;
        using co_handle = typename base_t::co_handle;

        static_assert(!std::is_same_v<ResultType, void>);

        struct end_iterator {};
        struct gen_iter {
        private:
            co_handle owner = nullptr; // never null, but not reference because must be non const + need default constructor for concept input_iterator
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = ResultType;
            using difference_type = ptrdiff_t; // ��������� ����, �� ��������� ��� concept input_iterator == true

            constexpr gen_iter() noexcept = default;
            explicit gen_iter(co_handle owner) noexcept
                : owner(owner) {
                owner.resume();
            }
            bool operator==(end_iterator) const noexcept {
                return owner.done();
            }
            bool operator!=(end_iterator) const noexcept {
                return !owner.done();
            }
            value_type& operator*() const noexcept {
                return owner.promise().storage;
            }
            gen_iter& operator++() {
                owner.resume();
                return *this;
            }
            // incorrect, but its impossible make it GOOD to return iterator that can * and get previous value of generator
            gen_iter& operator++(int) {
                owner.resume();
                return *this;
            }
        };

    public:
        using base_t::base_t;
        using iterator = gen_iter;
        using value_type = ResultType;

        value_type& Get() const noexcept {
            return this->my_handle.promise().storage;
        }
        value_type Next() {
            this->Resume();
            return Get();
        }

        iterator begin() noexcept {
            return iterator(this->my_handle);
        }
        end_iterator end() const noexcept {
            return end_iterator{};
        }

        auto operator co_await() const & noexcept {
            struct awaiter_t {
                const generator& my_generator;

                bool await_ready() const noexcept {
                    return false;
                }
                co_handle await_suspend(std::coroutine_handle<>) const noexcept {
                    return my_generator.my_handle;
                }
                value_type await_resume() const {
                    return my_generator.Get();
                }
            };
            return awaiter_t(*this);
        }
    };

    // concept-helper
    template<typename T>
    concept is_signature = requires(T) {
        typename T::func_type; // TODO better
    };

    template<typename ... Types>
    struct find_signature {
    private:
        template<size_t Number, typename First, typename ... Others>
        static consteval auto Finder(type_list<First, Others...>, std::integral_constant<size_t, Number>) noexcept {
            if constexpr (is_signature<First>) {
                struct result
                    : std::integral_constant<size_t, Number> {
                    using type = First;
                };
                return result{};
            }
            else {
                return Finder(type_list<Others...>{}, std::integral_constant<size_t, Number + 1>{});
            }
        }
        template<size_t N>
        static consteval auto Finder(type_list<>, std::integral_constant<size_t, N>) {
            static_assert(always_false<std::integral_constant<size_t, N>>(),
                "TODO Call usage example here You must give me signature of callback, for example kelbon::signature<int(const boost::error_code&, size_t)>");
        }
        using result_t = decltype(Finder(type_list<Types...>{}, std::integral_constant<size_t, 0>{}));
    public:
        using type = typename result_t::type;            // ���� ��������� 
        static constexpr size_t value = result_t::value; // ����� ��������� � ������ �����
    };

    template<typename ... Types>
    [[nodiscard]] consteval size_t sizeof_type_list(type_list<Types...>) {
        return sizeof_pack<Types...>();
    }

    template<typename F, typename ... Args>
    struct call {
    private:

        using cb_signature = typename find_signature<Args...>::type;
        static constexpr size_t signature_nb = find_signature<Args...>::value;
        using cb_args_tuple = insert_type_list_t<std::tuple, typename cb_signature::parameter_list>;
        //using result_type = std::invoke_result_t<F, Args...>; TODO

        F f;
        std::tuple<Args...> input_args;
        // TODO - ������� �� ������� �� ��� ��� ��������� ���(�� ��� � �� ������ ������������), ����� ��������� � ������ �����
        // [[no_unique_address]] std::conditional_t<!std::is_void_v<result_type>, result_type, nullstruct> result;

        std::byte output_args[sizeof(cb_args_tuple)]; // TODO ���� � ������� 0 ����������, �� ��� ����� �������� �� ���������� ���� ����

        // ������� ��������� �� ������ �������� �������������, �������� ������. �������� �� ��� ����� �����, � ������� ����� ��� ��� ��� ������

        struct awaiter_t {
            call& my_call;

            constexpr bool await_ready() noexcept {
                return false;
            }
            template<typename P>
            constexpr void await_suspend(std::coroutine_handle<P> handle) {
            // ��� ���������� ������ ������������ 2 ������ �������, ������� ����� ����� ������� �� ����� ��������� �� � ����� ���������, � ����� ���� �������� callback
                [this, handle] <size_t ... Is1, size_t ... Is2, typename ... CallBackArgs>
                    (value_list<size_t, Is1...>, value_list<size_t, Is2...>, type_list<CallBackArgs...>) -> decltype(auto)
                {
                    return std::invoke
                    (
                        my_call.f,                                                                      // ���������� �������
                        std::get<Is1>(my_call.input_args)...,                                           // ���������� ��������� �� ���������
                        [this, handle](CallBackArgs ... cb_args)                                        // ����������� ���� ������, ������������ �������� �� ������ ������
                        {
                            new (my_call.output_args) std::tuple<CallBackArgs...>(cb_args...);          // ��������� �������� ���������
                            handle.resume();                                                            // ��������� �������� ������ �������
                        },
                        std::get<Is2>(my_call.input_args)...                                            // ���������� ��������� ����� ���������
                            );
                }(                                                                                      // ���������� ����� ������ �� �����
                    make_value_list<size_t, signature_nb>{},                                            // �� ���������
                    make_value_list<size_t, sizeof...(Args) - signature_nb - 1, signature_nb + 1>{},    // ����� ���������
                    typename cb_signature::parameter_list{}                                             // ��������� �������
                        );
            }
            constexpr auto await_resume() {
                return std::move(*reinterpret_cast<cb_args_tuple*>(my_call.output_args));
            }
        };
    public:
        // TODO!!!!!!! �����, ����� �� ������� �������� � ����... Args1... signature<Ret(Args2...), Args3...

        constexpr call(F&& f, Args&& ... args) 
            : f(std::forward<F>(f)), input_args(std::forward<Args>(args)...)
        {}

        auto operator co_await() noexcept {
            return awaiter_t(*this);
        }
    };

} // namespace kelbon::coro

#endif // !KELBON_COROUTINES_HPP
