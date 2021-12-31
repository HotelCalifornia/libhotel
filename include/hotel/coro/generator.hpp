#include <coroutine>
#include <iterator>
#include <memory>

#include <cstddef>

#ifndef HOTEL_CORO_GENERATOR_HPP
#define HOTEL_CORO_GENERATOR_HPP

namespace hotel::coro {

        template<class T>
        class generator;

        namespace {
            template<class T>
            struct generator_promise_type {
                using value_type = std::remove_reference<T>;
                using reference_type = std::conditional_t<std::is_reference<T>::value, T, T &>;
                using pointer_type = value_type *;

                static auto get_return_object_on_allocation_failure() { return generator<T>{nullptr}; };

                auto get_return_object()

                noexcept;

                auto initial_suspend() { return std::suspend_always{}; };

                auto final_suspend()

                noexcept { return std::suspend_always{}; };

                void unhandled_exception() { _exception = std::current_exception(); };

                void return_void() {};

                auto yield_value(T value) {
                    current_value = value;
                    return std::suspend_always{};
                };

                void rethrow_if_exception() {
                    if (_exception) {
                        std::rethrow_exception(_exception);
                    }
                }

                T value() const

                noexcept { return current_value; };
            private:
                T current_value;
                std::exception_ptr _exception;
            };

            struct generator_sentinel {
            };

            template<class T>
            class generator_iterator {
                using handle = std::coroutine_handle<generator_promise_type<T>>;

                handle coro;
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = std::ptrdiff_t;
                using value_type = typename generator<T>::promise_type::value_type;
                using pointer = typename generator<T>::promise_type::pointer_type;

                generator_iterator() noexcept : coro(nullptr) {};

                explicit generator_iterator(handle c) : coro(c) {};

                friend bool operator==(const generator_iterator &it, generator_sentinel) {
                    return !it.coro || it.coro.done();
                };

                friend bool operator!=(const generator_iterator &it, generator_sentinel s) {
                    return !(it == s);
                };

                friend bool operator==(generator_sentinel s, const generator_iterator &it) {
                    return (it == s);
                }

                friend bool operator!=(generator_sentinel s, const generator_iterator &it) {
                    return it != s;
                };

                generator_iterator &operator++() {
                    coro.resume();
                    if (coro.done()) {
                        coro.promise().rethrow_if_exception();
                    }
                    return *this;
                };

                void operator++(int) {
                    (void) operator++();
                };

                T operator*() const

                noexcept { return coro.promise().value(); };

                pointer operator->() const

                noexcept { return std::addressof(operator*()); };
            };
        }

        /**
         * generator object
         *
         * this object isn't usually constructed directly. rather, instances of this class can be implicitly created from
         * coroutine functions:
         *
         * ```{.cpp}
         * // python-style `range` function, generates sequence of [start, end)
         * hotel::coro::generator<int> range(int start, int end, int step = 1) {
         *     int len = (end - start) / step + bool((end - start) % step);
         *     int iterations = 0;
         *     int current = start - step;
         *     while (iterations++ < len) {
         *         current += step;
         *         co_yield current;
         *     }
         * }
         *
         * // use
         * // [0, 20)
         * for (auto i : range(0, 20)) std::cout << "i: " << i << std::endl;
         * ```
         *
         * @tparam T type of value the generator will emit
         */
        template<class T>
        class generator {
        public:
            using promise_type = generator_promise_type<T>;
            using iterator = generator_iterator<T>;

            generator() noexcept : coro(nullptr) {};

            generator(generator const &) = delete;

            generator(generator &&rhs) : coro(rhs.coro) { rhs.coro = nullptr; };

            generator &operator=(generator other) noexcept {
                std::swap(coro, other.coro);
                return *this;
            }

            ~generator() { if (coro) coro.destroy(); };

            /**
             * advance the generator
             * @return `false` if the generator is finished
             */
            bool next() { return coro ? (coro.resume(), !coro.done()) : false; };

            /**
             * get an iterator pointing to the start of the sequence
             *
             * typically this won't be used directly. rather it is preferable to write code of the form
             * ```{.cpp}
             * hotel::coro::generator<int> sequence();
             *
             * //...
             *
             * for (auto& element : sequence()) {
             *     std::cout << element << std::endl;
             * }
             * ```
             *
             * @return the iterator
             */
            iterator begin() {
                if (coro) {
                    coro.resume();
                    if (coro.done()) {
                        coro.promise().rethrow_if_exception();
                    }
                }

                return iterator{coro};
            };

            /**
             * marks the end of a generator for range-based iteration
             *
             * like with `hotel::coro::generator<T>::begin`, typically this isn't used directly.
             *
             * @return sentinel marking the end of the sequence
             */
            generator_sentinel end() noexcept {
                return {};
            }
        private:
            using handle = std::coroutine_handle<promise_type>;

            friend class generator_promise_type<T>;

            explicit generator(handle h) : coro(h) {};
            handle coro;
        };

        namespace {
            // define this here now that generator is a complete type
            template<class T>
            auto generator_promise_type<T>::get_return_object() noexcept {
                using handle = std::coroutine_handle<generator_promise_type<T>>;
                return generator{handle::from_promise(*this)};
            };
        }

} // namespace hotel

#endif // HOTEL_CORO_GENERATOR_HPP
