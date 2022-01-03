#ifndef HOTEL_CONCEPTS_HPP
#define HOTEL_CONCEPTS_HPP

#include <concepts>

namespace hotel::concepts {

    /**
     * @concept hotel::concepts::is_ratio_like<>
     *
     * a type that acts like `std::ratio<...>`, with static members `num` and `den`
     *
     * @sa https://en.cppreference.com/w/cpp/numeric/ratio/ratio
     * @headerfile hotel/concepts.hpp
     */
    template <class T>
    concept is_ratio_like = requires {
        T::num;
        T::den;
    };

    /**
     * @concept hotel::concepts::Ratio<>
     *
     * a type that satisfies `hotel::concepts::is_ratio_like<T>`
     *
     * @headerfile hotel/concepts.hpp
     */
    template <class T>
    concept Ratio = is_ratio_like<T>;

    /**
     * @concept hotel::concepts::is_feedback_function<>
     *
     * this concept is satisfied if `F` is Invocable and the result of invoking it is convertible to `input_t`
     *
     * @sa https://en.cppreference.com/w/cpp/concepts/invocable
     * @sa https://en.cppreference.com/w/cpp/types/result_of
     * @sa https://en.cppreference.com/w/cpp/concepts/convertible_to
     *
     * @headerfile hotel/concepts.hpp
     */
    template <class input_t, class F>
    concept is_feedback_function = std::invocable<F> && std::is_convertible<std::invoke_result_t<F>, input_t>::value;

    /**
     * @concept hotel::concepts::FeedbackFunction<>
     *
     * a type that satisfies `hotel::concepts::is_feedback_function<input_t, F>`
     *
     * @sa hotel::pid_controller
     *
     * @headerfile hotel/concepts.cpp
     */
    template <class input_t, class F>
    concept FeedbackFunction = is_feedback_function<input_t, F>;

    /**
     * @concept hotel::concepts::is_settled_function<>
     *
     * this concept is satisfied if is a predicate function that accepts an `input_t` parameter (i.e. satisfies
     * `std::predicate<F, input_t>`)
     *
     * @sa https://en.cppreference.com/w/cpp/concepts/predicate
     *
     * @headerfile hotel/concepts.hpp
     */
    template <class input_t, class F>
    concept is_settled_function = std::predicate<F, input_t>;

    /**
     * @concept hotel::concepts::SettledFunction<>
     *
     * a type that satisfies `hotel::concepts::is_settled_function<input_t, F>`
     *
     * @sa hotel::pid_controller
     *
     * @headerfile hotel/concepts.cpp
     */
    template <class input_t, class F>
    concept SettledFunction = is_settled_function<input_t, F>;
}

#endif // HOTEL_CONCEPTS_HPP
