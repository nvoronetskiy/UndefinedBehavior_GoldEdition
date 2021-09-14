

#ifndef KELBON_COROUTINES_HPP
#define KELBON_COROUTINES_HPP

#include <coroutine>
#include <iterator>
#include <tuple>
#include <exception>

#include "kelbon_concepts_base.hpp"
#include "kelbon_concepts_functional.hpp"
#include "kelbon_type_traits_advanced.hpp"
#include "kelbon_utility.hpp"
#include "kelbon_memory_block.hpp"

namespace kelbon::coro {

    // TEMPLATE STRUCT base_promise_type
    struct prohibit_co_await {
        template<typename T>
        void await_transform(T) = delete;
    };

    template<typename T>
    struct return_block : return_block<void> {
        using result_type = T;

        result_type storage;
        // ПОЛНЫЙ ЗАПРЕТ НА return_value это такая хуета это пиздец, 99% что либо проебешь значение в мусор удалив корутину, либо потеряешь его в range_based loop,
        // либо обрабатывая его существование сделаешь миллиард доп проверок в итераторах
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

    template<
        bool IsImmediateStart,
        bool IsResumable = true,
        bool StopBeforeEnd = true,
        typename ResultType = void,
        typename ContextType = nullstruct,
        typename AwaitTransformer = nullstruct
    >
    struct base_promise_type
        : return_block<ResultType>, AwaitTransformer {
    private:
        using ret_block = return_block<ResultType>;
        using transform_block = AwaitTransformer;

        struct memory_block_for_context {
        private:
            memory_block<sizeof(ContextType), ::std::tuple> data;
        public:
            constexpr operator ContextType& () noexcept {
                return data.GetDataAs<ContextType>();
            }
        };
        using context_storage_t = std::conditional_t<::std::is_default_constructible_v<ContextType>, ContextType, memory_block_for_context>;

        [[no_unique_address]] context_storage_t my_context;
    public:
        using co_handle = ::std::coroutine_handle<base_promise_type>;
        static constexpr bool is_resumable = IsResumable;

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
        [[nodiscard]] constexpr ContextType& Context() noexcept {
            return my_context;
        }
        [[noreturn]] void unhandled_exception() {
            throw;
        }
    };

    // TEMPLATE CLASS FOR SPECIAL CONSTANTS this_coro_t
    enum class i_want : unsigned { context, promise_ptr };

    template<typename PromiseType, i_want What>
    struct this_coro_t {
        consteval this_coro_t() noexcept = default;

        this_coro_t(this_coro_t&&) = delete;
        this_coro_t(const this_coro_t&) = delete;
        void operator=(this_coro_t&&) = delete;
        void operator=(const this_coro_t&) = delete;

        // корутина может обращться с своим фреймом и промисом только через co_await по сути, так что чтобы обратится к своему же promise, который лежит на её фрейме,
        // нужно провести эту нетривиальную операцию. Но профит значительный, можно доставать промис и делать что хочешь
        constexpr [[nodiscard]] auto operator co_await() const noexcept {
            struct awaiter_t {
            private:
                mutable ::std::coroutine_handle<PromiseType> saved_handle;
            public:
                constexpr bool await_ready() const noexcept {
                    return false;
                }
                constexpr bool await_suspend(::std::coroutine_handle<PromiseType> handle) const noexcept {
                    saved_handle = handle;
                    return false;
                }
                constexpr decltype(auto) await_resume() const noexcept {
                    if constexpr (What == i_want::context) {
                        return saved_handle.promise().Context();    // reference to context
                    }
                    else if constexpr (What == i_want::promise_ptr) {
                        return &saved_handle.promise();             // pointer to promise
                    }
                }
            };
            return awaiter_t();
        }
    };

    // TEMPLATE ALIAS FOR BETTER self_context AND OTHER SPECIAL CONSTANTS USAGE
    template<callable auto F>
    using coroutine_from = func::result_type<decltype(F)>;

    // TEMPLATE coroutine
    template<typename PromiseType>
    struct coroutine {
        using promise_type = PromiseType;
        using co_handle = typename promise_type::co_handle;
    protected:
        co_handle my_handle;
    public:
        constexpr coroutine() noexcept = default;
        constexpr coroutine(co_handle handle)
            noexcept(::std::is_nothrow_copy_constructible_v<co_handle>)
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
            // TODO - эта штука только если suspended корутина не UB. Но т.к. предполагается исполнение из разных потоков, то... 
            return my_handle.done();
        }
        constexpr auto& Context() const noexcept {
            return my_handle.promise().Context();
        }

        static constexpr this_coro_t<promise_type, i_want::context> self_context = {};

        ~coroutine() {
            if (my_handle) {
                my_handle.destroy();
            }
        }
    };

    // TEMPLATE COROUTINE TYPE generator
    template<typename ResultType, typename ContextType = nullstruct, typename AwaitTransformer = nullstruct>
    struct generator
        : coroutine<base_promise_type<false, true, true, ResultType, ContextType, AwaitTransformer>> {
    private:
        using base_t = coroutine<base_promise_type<false, true, true, ResultType, ContextType, AwaitTransformer>>;
        using co_handle = typename base_t::co_handle;

        static_assert(!::std::is_same_v<ResultType, void>);

        struct end_iterator {};
        struct gen_iter {
        private:
            co_handle owner = nullptr; // never null, but not reference because must be non const + need default constructor for concept input_iterator
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = ResultType;
            using difference_type = ptrdiff_t; // очевидный бред, но требуется для concept input_iterator == true

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
            struct [[nodiscard]] awaiter_t {
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

    // TEMPLATE COROUTINE TYPE grasshopper
    template<typename ... ContextTypes>
    using grasshopper_promise_type = base_promise_type<false, false, true, void,
        ::std::conditional_t<sizeof...(ContextTypes) == 1, typename type_list<ContextTypes...>::template get_element<0>, std::tuple<ContextTypes...>>>;

    template<typename ... ContextTypes>
    struct grasshopper : coroutine<grasshopper_promise_type<ContextTypes...>> {
    private:
        using base_t = coroutine<grasshopper_promise_type<ContextTypes...>>;
    public:
        using base_t::base_t;

        void Run() noexcept {
            this->my_handle.resume();
        }
        //constexpr bool Done() const noexcept {
        //    return this->doned.load(std::memory_order::relaxed);
        //}
    };

    // HELPER for jump_on
    template<typename T> // TODO
    concept executor = true;// requires(T) { &T::execute; };

    // TEMPLATE FOR SPECIAL co_awaitable TYPES
    template<typename ExecutorType>
    requires executor<ExecutorType>
    struct jump_on {
    private:
        ExecutorType* exe_ptr;

        struct awaiter_t {
// TODO хорошо подумать сколько времени живёт этот awaiter, по моему больше или столько же сколько потенциальный временный executor
            ExecutorType* my_exe;
            constexpr bool await_ready() const noexcept {
                return false;
            }
            template<typename P>
            constexpr void await_suspend(::std::coroutine_handle<P> handle) const
                noexcept(noexcept(std::declval<ExecutorType*>()->execute([] {}))) {
                my_exe->execute([handle] {
                    handle.resume();
                    });
            }
            constexpr void await_resume() const noexcept {}
        };
    public:
        constexpr jump_on(ExecutorType&& exe) noexcept
            : exe_ptr(std::addressof(exe))
        {}

        constexpr auto operator co_await() const&& noexcept {
            return awaiter_t(exe_ptr);
        }
    };

    // HELPER for call template
    template<typename ... Types>
    struct find_signature {
    private:
        template<size_t Number, typename First, typename ... Others>
        static consteval auto Finder(type_list<First, Others...>, ::std::integral_constant<size_t, Number>) noexcept {
            if constexpr (is_instance_of_v<signature, First>) {
                struct result
                    : std::integral_constant<size_t, Number> {
                    using type = First;
                };
                return result{};
            }
            else {
                return Finder(type_list<Others...>{}, ::std::integral_constant<size_t, Number + 1>{});
            }
        }
        template<size_t N>
        static consteval auto Finder(type_list<>, ::std::integral_constant<size_t, N>) {
            static_assert(always_false<::std::integral_constant<size_t, N>>(),
                "Call usage example: auto [x, y] = co_await call(WRAP(func), func_args_before_callback"
                "kelbon::signature<int(const boost::error_code&, size_t)>, func_args_after_callback");
        }
        using result_t = decltype(Finder(type_list<Types...>{}, std::integral_constant<size_t, 0>{}));
    public:
        using type = typename result_t::type;            // сама сигнатура 
        static constexpr size_t value = result_t::value; // номер сигнатуры в списке типов
    };

// wraps any function/method/etc for better using CALL template, works also for overloaded functions etc
// you can capture what you want in "...", for example WRAP(socket.async_send, &socket, count)
#define WRAP(function_name, ...) \
    [__VA_ARGS__](auto&& ... KELBON__args) \
    { \
        function_name(std::forward<decltype(KELBON__args)>(KELBON__args)...); \
    }

        template<typename T>
    struct just_type {
    private:
        template<typename,typename...>
        friend struct call;

        T value;
        bool is_setted = false;

        // метод для awaiter_t и реализации
        constexpr T get() {
            if (!is_setted) {
                throw std::exception("Value for callback result is not setted, kelbon::coro::just_type");
            }
            return std::move(value);
        }
        constexpr void set(T v) noexcept(std::is_nothrow_move_assignable_v<T>) {
            value = std::move(v);
            is_setted = true;
        }
    public:
        // метод для корутины и пользователя
        constexpr void return_to_callback(T v) noexcept(std::is_nothrow_move_assignable_v<T>) {
            value = std::move(v);
            is_setted = true;
        }
    };

    template<typename T>
    struct bytes_for {
    private:
        std::byte data[sizeof(T)] = {};
        bool is_setted = false;

        template<typename,typename...>
        friend struct call;

        // метод для awaiter_t и реализации
        constexpr T get() {
            if (!is_setted) {
                throw std::exception("Value for callback result is not setted, kelbon::coro::bytes_for");
            }
            return std::move(*reinterpret_cast<T*>(data));
        }
        constexpr void set(T v) noexcept(std::is_nothrow_move_constructible_v<T>) {
            new(data) T(std::move(v));
            is_setted = true;
        }
    public:
        // метод для корутины и пользователя
        constexpr void return_to_callback(T v) noexcept(std::is_nothrow_move_constructible_v<T>) {
            new(data) T(std::move(v));
            is_setted = true;
        }
        ~bytes_for() {
            if (is_setted) {
                reinterpret_cast<T*>(data)->~T();
            }
        }
    };

    template<>
    struct just_type<void> : nullstruct {};
    template<>
    struct bytes_for<void> : nullstruct {};

    template<typename T>
    using storage_type = ::std::conditional_t<::std::is_default_constructible_v<T>, just_type<T>, bytes_for<T>>;

    // TEMPLATE FOR SPECIAL co_awaitable TYPES call
    template<typename F, typename ... Args>
    struct call {
    private:
        using cb_signature = decay_t<typename find_signature<Args...>::type>;
        static constexpr size_t signature_nb = find_signature<Args...>::value;
        using cb_args_tuple = insert_type_list_t<::std::tuple, typename cb_signature::parameter_list>;
        using cb_result_type = typename cb_signature::result_type;
        using cb_result_storage = storage_type<cb_result_type>;

        using result_args_tuple = ::std::conditional_t<::std::is_void_v<cb_result_type>,
            cb_args_tuple,
            insert_type_list_t<::std::tuple, merge_type_lists_t<typename cb_signature::parameter_list, type_list<cb_result_storage*>>>>;

        F f;
        ::std::tuple<Args...> input_args;
        // вместо добавления сюда type erase я сам удалю то что сюда добавил в деструкторе
        storage_type<result_args_tuple> output_args; // если результат void, то size_of == 0 // TODO если у калбека 0 аргументов, то мне нужно очевидно не запоминать этот тупл

        // входные аргументы не всегда дефолтно конструируемы, например ссылки. получить их мне нужно потом, а создать место под них уже сейчас

        struct awaiter_t {
            call& my_call;
            // TODO - перенести всё в awaiter(output input args и т.д.), вместо класса call будет функция такая
            constexpr bool await_ready() noexcept {
                return false;
            }
            template<typename P>
            constexpr void await_suspend(::std::coroutine_handle<P> handle) {

                [this, handle] <size_t ... Is1, size_t ... Is2, typename ... CallBackArgs>
                    (value_list<size_t, Is1...>, value_list<size_t, Is2...>, type_list<CallBackArgs...>) -> decltype(auto)
                {
                    return std::invoke
                    (
                        my_call.f,                                                                      // вызываемая функция
                        std::get<Is1>(my_call.input_args)...,                                           // переданные аргументы до сигнатуры
                        [this, handle](CallBackArgs ... cb_args)                                        // формируемый мной калбек, пробуждающий корутину на нужном потоке
                        {
                            if constexpr (std::is_void_v<cb_result_type>) {
                                my_call.output_args.set(::std::tuple<CallBackArgs...>(cb_args...));       // запоминаю входящие аргументы
                                handle.resume();                                                        // пробуждаю корутину внутри калбека
                            }
                            else {
                                cb_result_storage result;
                                my_call.output_args.set(::std::tuple<CallBackArgs..., cb_result_storage*>(cb_args..., &result));
                                handle.resume();
                                return result.get(); // когда корутина в следующий раз отдаст контроль тут продолжится выполнение и значение вернётся
                            }
                        },
                        std::get<Is2>(my_call.input_args)...                                            // переданные аргументы после сигнатуры
                            );
                }(                                                                                      // МГНОВЕННЫЙ ВЫЗОВ ЛЯМБДЫ НА МЕСТЕ
                    make_value_list<size_t, signature_nb>{},                                            // до сигнатуры
                    make_value_list<size_t, sizeof...(Args) - signature_nb - 1, signature_nb + 1>{},    // после сигнатуры
                    typename cb_signature::parameter_list{}                                             // аргументы калбека
                        );
            }
            constexpr result_args_tuple await_resume() {
                return my_call.output_args.get();
            }
        };
    public:
        // конструктор шаблонный, чтобы принимать всё, иначе ожидается rvalue
        constexpr call(F&& f, Args&& ... args)
            : f(::std::forward<F>(f)), input_args(::std::forward<Args>(args)...), output_args{}
        {}

        auto operator co_await() && noexcept {
            return awaiter_t(*this);
        }
    };

    // deduction guide удивительно, но без него автоматически сгенерированный работает хуже
    template<typename ... Types>
    call(Types&&...)->call<Types&&...>;

} // namespace kelbon::coro

#endif // !KELBON_COROUTINES_HPP
