#include "test_common.hpp"

#include "mkn/kul/threads.hpp"

#include <atomic>
#include <chrono>
#include <future>

using Pool = mkn::kul::ThreadPool<>;

TEST(ThreadPool, RunsEveryTask) {
  std::atomic<size_t> n{0};
  Pool pool(4);
  for (size_t i = 0; i < 1000; ++i) EXPECT_TRUE(pool.async([&]() { ++n; }));
  pool.finish().join();
  EXPECT_EQ(n, 1000u);
  EXPECT_FALSE(pool.exception());
}

TEST(ThreadPool, TasksQueuedBeforeStartRun) {
  std::atomic<size_t> n{0};
  Pool pool(2, false);
  for (size_t i = 0; i < 10; ++i) pool.async([&]() { ++n; });
  pool.start().finish().join();
  EXPECT_EQ(n, 10u);
}

TEST(ThreadPool, RunsConcurrently) {
  auto const start = std::chrono::steady_clock::now();
  Pool pool(4);
  for (size_t i = 0; i < 4; ++i)
    pool.async([]() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
  pool.finish().join();
  EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(300));
}

TEST(ThreadPool, NoPollingLatency) {
  // each task is queued only after the previous one ran, so any dispatch polling
  // interval would be paid every time
  Pool pool(1);
  auto const start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < 200; ++i) {
    std::promise<void> ran;
    pool.async([&]() { ran.set_value(); });
    ran.get_future().wait();
  }
  EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(100));
}

TEST(ThreadPool, OnErrorHandlesAndPoolContinues) {
  std::atomic<size_t> errors{0}, n{0};
  Pool pool(2);
  pool.async([]() { KEXCEPTION("handled"); }, [&](mkn::kul::Exception const&) { ++errors; });
  for (size_t i = 0; i < 10; ++i) pool.async([&]() { ++n; });
  pool.finish().join();
  EXPECT_EQ(errors, 1u);
  EXPECT_EQ(n, 10u);
  EXPECT_FALSE(pool.exception());
}

TEST(ThreadPool, UnhandledExceptionIsKeptAndPoolContinues) {
  std::atomic<size_t> n{0};
  Pool pool(1);
  pool.async([]() { KEXCEPTION("unhandled"); });
  for (size_t i = 0; i < 10; ++i) pool.async([&]() { ++n; });
  pool.finish().join();
  EXPECT_EQ(n, 10u);
  EXPECT_TRUE(pool.exception());
  EXPECT_THROW(pool.rethrow(), mkn::kul::Exception);
}

TEST(ThreadPool, OnErrorCanStopPool) {
  std::atomic<size_t> n{0};
  std::promise<void> release;
  auto gate = release.get_future().share();
  Pool pool(1);
  pool.async([gate]() {
    gate.wait();
    KEXCEPTION("stop");
  }, [&](mkn::kul::Exception const&) { pool.stop(); });
  for (size_t i = 0; i < 10; ++i) pool.async([&]() { ++n; });
  release.set_value();
  pool.join();
  EXPECT_EQ(n, 0u);
  EXPECT_FALSE(pool.exception());
}

TEST(ThreadPool, ThrowingOnErrorBecomesPoolException) {
  Pool pool(1);
  pool.async([]() { KEXCEPTION("first"); },
             [](mkn::kul::Exception const& e) { throw e; });
  pool.finish().join();
  EXPECT_THROW(pool.rethrow(), mkn::kul::Exception);
}

TEST(ThreadPool, NonKulExceptionIsKept) {
  Pool pool(1);
  pool.async([]() { throw std::runtime_error("std"); });
  pool.finish().join();
  EXPECT_THROW(pool.rethrow(), std::runtime_error);
}

TEST(ThreadPool, BlockRejectsNewTasks) {
  Pool pool(1);
  pool.block();
  EXPECT_FALSE(pool.async([]() {}));
  pool.finish().join();
}

TEST(ThreadPool, StopFromTaskDoesNotDeadlock) {
  Pool pool(2);
  pool.async([&]() { pool.stop(); });
  pool.join();
  EXPECT_FALSE(pool.running());
}

TEST(ThreadPool, StopDropsQueuedTasks) {
  std::atomic<size_t> n{0};
  std::promise<void> release;
  auto gate = release.get_future().share();
  Pool pool(1);
  pool.async([&, gate]() { gate.wait(); });
  for (size_t i = 0; i < 10; ++i) pool.async([&]() { ++n; });
  pool.stop();
  release.set_value();
  pool.join();
  EXPECT_EQ(n, 0u);
}
