//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++03, c++98, c++11, c++14

// <shared_mutex>

// class shared_mutex;

// void lock();

#include <shared_mutex>
#include <thread>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

std::shared_mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

ms WaitTime = ms(250);

// Thread sanitizer causes more overhead and will sometimes cause this test
// to fail. To prevent this we give Thread sanitizer more time to complete the
// test.
#if !TEST_HAS_FEATURE(thread_sanitizer)
ms Tolerance = ms(50);
#else
ms Tolerance = ms(100);
#endif

void f()
{
    time_point t0 = Clock::now();
    m.lock();
    time_point t1 = Clock::now();
    m.unlock();
    ns d = t1 - t0 - WaitTime;
    assert(d < Tolerance);  // within tolerance
}

int main()
{
    m.lock();
    std::thread t(f);
    std::this_thread::sleep_for(WaitTime);
    m.unlock();
    t.join();
}
