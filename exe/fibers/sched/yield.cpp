#include "yield.hpp"


namespace exe::fibers {

void Yield() {
  class YieldAwaiter : public Awaiter {
   public:
    void RunAwaiter(FiberHandle handle) noexcept override {
      handle.Schedule();
    }
    virtual ~YieldAwaiter() = default;
  };

  YieldAwaiter awaiter;
  exe::fibers::Suspend(&awaiter);
}

}  // namespace exe::fiber
