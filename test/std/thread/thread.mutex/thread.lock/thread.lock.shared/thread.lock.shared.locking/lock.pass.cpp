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
// UNSUPPORTED: c++98, c++03, c++11

// <shared_mutex>

// template <class Mutex> class shared_lock;

// void lock();

#include <shared_mutex>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

std::shared_timed_mutex m;

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
ms Tolerance = ms(25);
#else
ms Tolerance = ms(75);
#endif


void f()
{
    std::shared_lock<std::shared_timed_mutex> lk(m, std::defer_lock);
    time_point t0 = Clock::now();
    lk.lock();
    time_point t1 = Clock::now();
    assert(lk.owns_lock() == true);
    ns d = t1 - t0 - WaitTime;
    assert(d < Tolerance);  // within tolerance
    try
    {
        lk.lock();
        assert(false);
    }
    catch (std::system_error& e)
    {
        assert(e.code().value() == EDEADLK);
    }
    lk.unlock();
    lk.release();
    try
    {
        lk.lock();
        assert(false);
    }
    catch (std::system_error& e)
    {
        assert(e.code().value() == EPERM);
    }
}

int main()
{
    m.lock();
    std::vector<std::thread> v;
    for (int i = 0; i < 5; ++i)
        v.push_back(std::thread(f));
    std::this_thread::sleep_for(WaitTime);
    m.unlock();
    for (auto& t : v)
        t.join();
}
