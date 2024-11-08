#pragma once


#include <cstdlib>
#include <span>
#include <utility>

namespace syscalls::stacks {

using Stack = NewStack;

class NewStack {
 private:
  struct Alloc {
    std::byte* start = nullptr;
    std::size_t size = 0u;
  };

 public:
  NewStack() = delete;

  // Non-copyable
  NewStack(const NewStack&) = delete;
  NewStack& operator=(const NewStack&) = delete;

  // Move-constructible
  NewStack(NewStack&& that)
      : alloc_(std::exchange(that.alloc_, {nullptr, 0})) {
  }

  // Move-assignable
  NewStack& operator=(NewStack that) {
    Swap(that);
    return *this;
  }

  static NewStack AllocateBytes(size_t at_least) {
    size_t size = at_least;
    return NewStack{Alloc{new std::byte[size], size}};
  }

  void* LowerBound() const {
    return alloc_.start;
  }

  std::span<std::byte> MutView() {
    return {alloc_.start, alloc_.size};
  }

  ~NewStack() {
    delete[] alloc_.start;
  }

 private:
  NewStack(Alloc alloc)
      : alloc_(alloc) {
  }

  void Swap(NewStack& that) {
    std::swap(alloc_, that.alloc_);
  }

 private:
  Alloc alloc_;
};

}