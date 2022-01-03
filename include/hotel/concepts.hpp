#ifndef HOTEL_CONCEPTS_HPP
#define HOTEL_CONCEPTS_HPP

namespace hotel::concepts {
    template <class T>
    concept is_ratio_like =
    requires {
        T::num;
        T::den;
    };

    template <class T>
    concept Ratio = is_ratio_like<T>;

    template <class input_t, class F>
    concept is_feedback_function = std::is_convertible<std::invoke_result_t<F>, input_t>::value;

    template <class input_t, class F>
    concept FeedbackFunction = is_feedback_function<input_t, F>;

    template <class input_t, class F>
    concept is_settled_function = std::predicate<F, input_t>;

    template <class input_t, class F>
    concept SettledFunction = is_settled_function<input_t, F>;
}

#endif // HOTEL_CONCEPTS_HPP
