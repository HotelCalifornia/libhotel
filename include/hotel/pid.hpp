#include <chrono>
#include <concepts>
#include <functional>
#include <ratio>
#include <type_traits>

#include <cstdint>

#include "pros/rtos.hpp"

#include "hotel/concepts.hpp"
#include "hotel/coro/generator.hpp"

#ifndef HOTEL_PID_HPP
#define HOTEL_PID_HPP

namespace hotel {

    /**
     * PID controller object
     *
     * @tparam Kp_t `std::ratio` representing the proportional gain
     * @tparam Ki_t `std::ratio` representing the integral gain
     * @tparam Kd_t `std::ratio` representing the derivative gain
     * @tparam _output_t generator output type (should match whatever is being controlled)
     * @tparam FeedbackFn type representing a feedback function (should have the form `_target_t(*)()`,
     *                    `std::function<_target_t()>`, or equivalent)
     * @tparam _target_t setpoint type (deduced from `FeedbackFn` return type)
     * @tparam SettledFn type representing a function that evaluates whether the controller has settled (should have the
     *                   form `bool(*)(_target_t)`, `std::function<bool(_target_t)>`, or equivalent)
     */
    template <
        concepts::Ratio Kp_t, concepts::Ratio Ki_t, concepts::Ratio Kd_t,
        class _output_t,
        class FeedbackFn, class _target_t = typename std::result_of<FeedbackFn&()>::type,
        class SettledFn = std::function<bool(_target_t)>
    >
        requires concepts::FeedbackFunction<_target_t, FeedbackFn> && concepts::SettledFunction<_target_t, SettledFn>
    class pid_controller {
        _target_t current_setpoint;
        _target_t error_accumulator;
        _target_t last_error;
        std::chrono::time_point<pros::Clock> last_iteration;

        static constexpr float Kp = Kp_t::num / static_cast<float>(Kp_t::den);
        static constexpr float Ki = Ki_t::num / static_cast<float>(Ki_t::den);
        static constexpr float Kd = Kd_t::num / static_cast<float>(Kd_t::den);

        FeedbackFn feedback_fn;
        SettledFn is_settled;
    public:
        using target_t = _target_t;
        using output_t = _output_t;

        /**
         * construct a PID controller object
         *
         * typically it will be more convenient to use one of the typedefs (see below) rather than specifying the full
         * template for this type.
         *
         * example:
         * ```{.cpp}
         * pros::Motor motor{1};
         *
         * // use motor_position_controller instead of
         * //  pid_controller<std::ratio<...>, std::ratio<...>, std::ratio<...>, std::int32_t, std::function<double()>>
         * hotel::motor_position_controller<std::ratio<1, 2>, std::ratio<0, 1>, std::ratio<1, 100>> motor_controller{
         *     [&motor] { return motor.get_position(); },           // feedback function
         *     [] (double error) { return fabs(error) < 5; },       // settled function
         *     200.0                                                // initial setpoint (optional)
         * };
         * ```
         *
         * @param ffn a feedback function (of the form `output_t (*)()` or `std::function<output_t()>`
         * @param sfn a predicate function that returns true when
         * @param setpoint the initial setpoint for the controller
         */
        pid_controller(FeedbackFn ffn, SettledFn sfn, target_t setpoint = 0) :
            feedback_fn(ffn),
            is_settled(sfn),
            current_setpoint(setpoint),
            error_accumulator(0),
            last_error(0),
            last_iteration(pros::Clock::now()) {};

        /**
         * create PID function as a generator coroutine
         *
         * this function returns a generator coroutine that produces values according to a PID function with the
         * gains specified when this controller was created. the generator finishes when the settled function evaluates
         * to `true`.
         *
         * example:
         * ```{.cpp}
         * // run until within our defined threshold of initial setpoint
         * for (const std::int32_t& output : motor_controller.run()) {
         *     motor.move(std::clamp(output, std::int32_t{-127}, std::int32_t{127}));
         *     pros::delay(20);
         * }
         * ```
         *
         * @return the output according to @f$K_p * e(T) + K_i * \int_0^T e(T)dT + K_d * \frac{dE}{dT}@f$
         */
        coro::generator<output_t> run() {
            while (true) {
                auto error = current_setpoint - feedback_fn();

                if (is_settled(error)) {
                    break;
                }

                error_accumulator += error;

                auto now = pros::Clock::now();
                auto dT = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_iteration).count();

                auto value = static_cast<output_t>(Kp * error + Ki * error_accumulator * dT + Kd * ((last_error - error) / (dT)));

                last_iteration = now;
                last_error = error;

                co_yield value;
            }
        };

        /**
         * set a new target for this controller
         *
         * example:
         * ```{.cpp}
         * // grant new setpoint and run
         * for (const std::int32_t& output : motor_controller.target(300.0).run()) {
         *     motor.move(std::clamp(output, std::int32_t{-127}, std::int32_t{127}));
         *     pros::delay(10);
         * }
         * ```
         *
         * @param setpoint the new setpoint
         * @return this instance
         */
        pid_controller& target(target_t setpoint) {
            current_setpoint = setpoint;
            error_accumulator = 0;
            last_error = 0;
            last_iteration = pros::Clock::now();

            return *this;
        };
    };

    /**
     * typedef describing a position PID controller for a `pros::Motor` (feedback function e.g. `pros::Motor::get_position`)
     */
    template <concepts::Ratio Kp, concepts::Ratio Ki, concepts::Ratio Kd>
    using motor_position_controller = pid_controller<Kp, Ki, Kd, std::int32_t, std::function<double()>>;

    /**
     * typedef describing a velocity PID controller for a `pros::Motor` (feedback function e.g. `pros::Motor::get_actual_velocity`)
     */
    template <concepts::Ratio Kp, concepts::Ratio Ki, concepts::Ratio Kd>
    using motor_velocity_controller = pid_controller<Kp, Ki, Kd, std::int32_t, std::function<double()>>;

    /**
     * typedef describing a torque PID controller for a `pros::Motor` (feedback function e.g. `pros::Motor::get_current_draw`)
     */
    template <concepts::Ratio Kp, concepts::Ratio Ki, concepts::Ratio Kd>
    using motor_torque_controller = pid_controller<Kp, Ki, Kd, std::int32_t, std::function<std::int32_t()>>;

    /**
     * typedef describing a voltage PID controller for a `pros::Motor` (feedback function e.g. `pros::Motor::get_voltage`)
     */
    template <concepts::Ratio Kp, concepts::Ratio Ki, concepts::Ratio Kd>
    using motor_voltage_controller = pid_controller<Kp, Ki, Kd, std::int32_t, std::function<double()>>;
}

#endif // HOTEL_PID_HPP
