

#ifndef KELBON_COROUTINES_HPP
#define KELBON_COROUTINES_HPP

#include <coroutine>
#include <iterator>
#include <tuple>

#include "kelbon_concepts_base.hpp"
#include "kelbon_concepts_functional.hpp"
#include "kelbon_type_traits_advanced.hpp"
#include "kelbon_utility.hpp"

namespace kelbon::coro {

    template<typename T>
    struct just_type {
        T value;
        bool is_setted = false;

        // метод для awaiter_t и реализации
        constexpr T get() {
            if (!is_setted) {
                throw bad_memory_block_access("Value for callback result is not setted, kelbon::coro::just_type");
            }
            return std::move(value);
        }
        // метод для корутины и пользователя
        constexpr void return_to_callback(T v) noexcept(std::is_nothrow_move_assignable_v<T>) {
            value = std::move(v);
            is_setted = true;
        }
    };

    template<typename T>
    struct bytes_for {
        std::byte data[sizeof(T)];
        bool is_setted = false;

        // метод для awaiter_t и реализации
        constexpr T get() {
            if (!is_setted) {
                throw bad_memory_block_access("Value for callback result is not setted, kelbon::coro::bytes_for");
            }
            return std::move(*(reinterpret_cast<T*>(data)));
        }
        // метод для корутины и пользователя
        constexpr void return_to_callback(T v) noexcept(std::is_nothrow_move_constructible_v<T>) {
            new(data) T(std::move(v));
            is_setted = true;
        }
    };

    template<typename T>
    using storage_type = std::conditional_t<std::is_default_constructible_v<T>, just_type<T>, bytes_for<T>>;

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
    
    // concept-helper
    template<typename T>
    concept some_default_constructible_std_tuple = is_instance_of_v<std::tuple, T> && std::is_default_constructible_v<T>;


    // TEMPLATE STRUCT base_promise_type
    template<
        bool IsImmediateStart,
        bool IsResumable = true,
        bool StopBeforeEnd = true,
        typename ResultType = void,
        some_default_constructible_std_tuple ContextType = std::tuple<>,
        typename AwaitTransformer = nullstruct
    >
    struct base_promise_type
        : return_block<ResultType>, AwaitTransformer {
    private:
        using ret_block = return_block<ResultType>;
        using transform_block = AwaitTransformer;

        template<typename T>
        struct no_copyble_storage {
        private:
            T value;
        public:
            no_copyble_storage() = default;
            no_copyble_storage(const no_copyble_storage&) = delete;
            void operator=(const no_copyble_storage&) = delete;
            constexpr T& Get() noexcept {
                return value;
            }
        };
        [[no_unique_address]] no_copyble_storage<ContextType> my_context; // TODO - проверить ломает ли этот аттрибут что то, если не 0 размер
    public:
        using co_handle = std::coroutine_handle<base_promise_type>;
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
        [[nodiscard]] constexpr auto& Context() noexcept requires(!std::same_as<std::tuple<>, ContextType>) {
            return my_context.Get();
        }
        [[noreturn]] void unhandled_exception() {
            throw;
        }
    };

    template<typename PromiseType, bool GetContext = false>
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
                mutable std::coroutine_handle<PromiseType> saved_handle;
            public:
                constexpr bool await_ready() const noexcept {
                    return false;
                }
                constexpr bool await_suspend(std::coroutine_handle<PromiseType> handle) const noexcept {
                    saved_handle = handle;
                    return false;
                }
                constexpr auto& await_resume() const noexcept { // requires(GetContext) {
                    return saved_handle.promise().Context();   // reference to context
                }
                //constexpr auto await_resume() noexcept requires(!GetContext) {
                //    return &saved_handle.promise();             // pointer to promise
                //}
            };
            return awaiter_t();
        }
    };

    template<callable auto F>
    using coro_type = func::result_type<decltype(F)>;

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
            // TODO - эта штука только если suspended корутина не UB. Но т.к. предполагается исполнение из разных потоков, то... 
            return my_handle.done();
        }
        constexpr auto& Context() const noexcept {
            return my_handle.promise().Context();
        }

        static constexpr this_coro_t<promise_type> self_context = {};

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

    template<typename ... Types>
    struct find_signature {
    private:
        template<size_t Number, typename First, typename ... Others>
        static consteval auto Finder(type_list<First, Others...>, std::integral_constant<size_t, Number>) noexcept {
            if constexpr (is_instance_of_v<signature, First>) {
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
        using type = typename result_t::type;            // сама сигнатура 
        static constexpr size_t value = result_t::value; // номер сигнатуры в списке типов
    };

    // TODO - sizeof для всех возможных вариантов объединитьв один/ два
    template<typename ... Types>
    [[nodiscard]] consteval size_t sizeof_type_list(type_list<Types...>) {
        return sizeof_pack<Types...>();
    }

    // COROUTINE TYPE grasshopper
    template<typename ... ContextTypes>
    using grasshopper_promise_type = base_promise_type<false, false, true, void, std::tuple<ContextTypes...>>;

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

    template<typename T>
    concept executor = true;// requires(T) { &T::execute; };

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
            constexpr void await_suspend(std::coroutine_handle<P> handle) const
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

    template<typename F, typename ... Args>
    struct call {
    private:
        using cb_signature = typename find_signature<Args...>::type;
        static constexpr size_t signature_nb = find_signature<Args...>::value;
        using cb_args_tuple = insert_type_list_t<std::tuple, typename cb_signature::parameter_list>;
        using cb_result_type = typename cb_signature::result_type;
        using cb_result_storage = storage_type<cb_result_type>;

        using result_args_tuple = std::conditional_t<std::is_void_v<cb_result_type>,
            cb_args_tuple,
            insert_type_list_t<std::tuple, merge_type_lists_t<typename cb_signature::parameter_list, type_list<cb_result_storage*>>>>;

        F f;
        std::tuple<Args...> input_args;
        std::byte output_args[sizeof(result_args_tuple)]; // если результат void, то size_of == 0 // TODO если у калбека 0 аргументов, то мне нужно очевидно не запоминать этот тупл

        // входные аргументы не всегда дефолтно конструируемы, например ссылки. получить их мне нужно потом, а создать место под них уже сейчас

        struct [[nodiscard]] awaiter_t {
            call& my_call;

            constexpr bool await_ready() noexcept {
                return false;
            }
            template<typename P>
            constexpr void await_suspend(std::coroutine_handle<P> handle) {
            // Тут вызывается лямбда раскрывающая 2 индекс массива, которые нужны чтобы достать из тупла аргументы ДО и ПОСЛЕ сигнатуры, а между ними вставить callback
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
                                new(my_call.output_args) std::tuple<CallBackArgs...>(cb_args...);       // запоминаю входящие аргументы
                                handle.resume();                                                        // пробуждаю корутину внутри калбека
                            }
                            else {
                                cb_result_storage result;
                                new(my_call.output_args) std::tuple<CallBackArgs..., cb_result_storage*>(cb_args..., &result);
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
                return std::move(*reinterpret_cast<result_args_tuple*>(my_call.output_args));
            }
        };
    public:
        // TODO!!!!!!! блять, можно же сделать красивее в разы... Args1... signature<Ret(Args2...), Args3...
        constexpr call(F&& f, Args&& ... args) 
            : f(std::forward<F>(f)), input_args(std::forward<Args>(args)...), output_args{}
        {}

        auto operator co_await() && noexcept {
            return awaiter_t(*this);
        }
    };

} // namespace kelbon::coro

#endif // !KELBON_COROUTINES_HPP
