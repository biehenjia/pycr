#include <chrono>

#ifdef CREVAL_PROFILE
struct _CrevalScopedTimer {
    using Clock = std::chrono::steady_clock;
    using Ms    = std::chrono::duration<double, std::milli>;
    double& acc;
    Clock::time_point t0;
    explicit _CrevalScopedTimer(double& a) : acc(a), t0(Clock::now()) {}
    ~_CrevalScopedTimer() { acc += Ms(Clock::now() - t0).count(); }
};
#define CREVAL_TIME_BLOCK(VAR) _CrevalScopedTimer _scoped_timer_##__LINE__{VAR}
#else
#define CREVAL_TIME_BLOCK(VAR) if (false) for (; false; )
#endif