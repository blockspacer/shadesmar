/* MIT License

Copyright (c) 2020 Dheeraj R Reddy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

#ifndef INCLUDE_SHADESMAR_CONCURRENCY_LOCKLESS_SET_H_
#define INCLUDE_SHADESMAR_CONCURRENCY_LOCKLESS_SET_H_

#include <cstring>

#include <array>
#include <atomic>

#include "shadesmar/macros.h"

#ifdef __APPLE__
#define __pid_t __darwin_pid_t
#endif

namespace shm::concurrent {
class LocklessSet {
 public:
  LocklessSet();
  LocklessSet &operator=(const LocklessSet &);

  bool insert(uint32_t elem);
  bool remove(uint32_t elem);

  std::array<std::atomic_uint32_t, MAX_SHARED_OWNERS> __array = {};
};

LocklessSet::LocklessSet() = default;

LocklessSet &LocklessSet::operator=(const LocklessSet &set) {
  for (uint32_t idx = 0; idx < MAX_SHARED_OWNERS; ++idx) {
    __array[idx].store(set.__array[idx].load());
  }
  return *this;
}

bool LocklessSet::insert(uint32_t elem) {
  for (uint32_t idx = 0; idx < MAX_SHARED_OWNERS; ++idx) {
    auto probedElem = __array[idx].load();

    if (probedElem != elem) {
      if (probedElem != 0) {
        continue;
      }
      uint32_t exp = 0;
      if (__array[idx].compare_exchange_strong(exp, elem)) {
        return true;
      } else {
        continue;
      }
    }
    return false;
  }
  return false;
}

bool LocklessSet::remove(uint32_t elem) {
  for (uint32_t idx = 0; idx < MAX_SHARED_OWNERS; ++idx) {
    auto probedElem = __array[idx].load();

    if (probedElem == elem) {
      return __array[idx].compare_exchange_strong(elem, 0);
    }
  }

  return false;
}
}  // namespace shm::concurrent

#endif  // INCLUDE_SHADESMAR_CONCURRENCY_LOCKLESS_SET_H_
