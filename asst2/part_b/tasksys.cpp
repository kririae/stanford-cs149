#include "tasksys.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <cassert>
#include <iostream>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() { return "Serial"; }

TaskSystemSerial::TaskSystemSerial(int num_threads)
    : ITaskSystem(num_threads) {}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable,
                                          int        num_total_tasks,
                                          const std::vector<TaskID>& deps) {
  run(runnable, num_total_tasks);
  return 0;
}

void TaskSystemSerial::sync() { return; }

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
  return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads)
    : ITaskSystem(num_threads), num_threads(num_threads) {
  //
  // TODO: CS149 student implementations may decide to perform setup
  // operations (such as thread pool construction) here.
  // Implementations are free to add new class member variables
  // (requiring changes to tasksys.h).
  //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
  //
  // CS149 students will modify the implementation of this
  // method in Part A.  The implementation provided below runs all
  // tasks sequentially on the calling thread.
  //
  std::vector<std::thread> workers{};

  // serves as the pointer to the currently waiting thread, starting from 0
  int        num_threads_cur = 0;
  std::mutex launching_task{};

  auto worker = [&](int worker_id) -> void {
    // When launching the task, others cannot take over the task
    while (true) {
      int task_id;
      launching_task.lock();

      if (num_threads_cur >= num_total_tasks) {
        launching_task.unlock();
        break;
      }

      task_id = num_threads_cur;
      num_threads_cur++;
      // printf("launching the %d task from the %d worker\n", task_id,
      // worker_id);

      launching_task.unlock();

      runnable->runTask(task_id, num_total_tasks);
    }
  };

  for (int i = 0; i < num_threads; ++i) {
    workers.emplace_back(worker, i);
  }

  for (int i = 0; i < num_threads; ++i) {
    workers[i].join();
  }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(
    IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps) {
  run(runnable, num_total_tasks);
  return 0;
}

void TaskSystemParallelSpawn::sync() { return; }

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
  return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(
    int num_threads)
    : ITaskSystem(num_threads), num_threads(num_threads) {
  //
  // CS149 student implementations may decide to perform setup
  // operations (such as thread pool construction) here.
  // Implementations are free to add new class member variables
  // (requiring changes to tasksys.h).
  //

  auto worker = [&](int worker_id) {
    // Spinning
    while (true) {
      int task_id;

      if (halt_flag) {
        // halt the spinning
        break;
      }

      launching_task.lock();
      if (tasks.empty()) {
        launching_task.unlock();
        continue;
      }

      task_id = tasks.front();
      tasks.pop();
      num_acc_tasks++;

      launching_task.unlock();

      // Launch tasks from `global_runnable`
      assert(global_runnable != nullptr);
      global_runnable->runTask(task_id, global_num_total_tasks);
      num_fin_tasks++;
    }
  };

  // Thread pool setup here
  for (int i = 0; i < num_threads; ++i) {
    workers.emplace_back(worker, i);
  }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
  halt_flag = true;
  for (int i = 0; i < num_threads; ++i) {
    workers[i].join();
  }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable,
                                               int        num_total_tasks) {
  //
  // TODO: CS149 students will modify the implementation of this
  // method in Part A.  The implementation provided below runs all
  // tasks sequentially on the calling thread.
  //

  assert(global_runnable == nullptr);
  assert(num_fin_tasks == 0);
  global_runnable        = runnable;
  global_num_total_tasks = num_total_tasks;

  // pushed tasks into queue
  launching_task.lock();
  for (int i = 0; i < num_total_tasks; ++i) tasks.push(i);
  launching_task.unlock();

  while (num_fin_tasks != num_total_tasks) {
    // Waiting all tasks to be finished
    continue;
  }

  assert(num_fin_tasks == num_total_tasks);

  // join those threads
  global_runnable        = nullptr;
  global_num_total_tasks = 0;
  num_fin_tasks          = 0;
  num_acc_tasks          = 0;
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(
    IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps) {
  run(runnable, num_total_tasks);
  return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() { return; }

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
  return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(
    int num_threads)
    : ITaskSystem(num_threads), num_threads(num_threads) {
  // Thread worker, keep active until there's no task to
  // task to fetch from the queue `tasks`
  auto worker = [&](int worker_id) {
    Task task;
    while (true) {
      // Wake state
      if (halt_flag) break;

      // Thread sleep state
      std::unique_lock<std::mutex> lk(operate_queue);

      if (tasks.empty()) {
        // Sleep state
        wake.wait(lk);  // acquire the mutex
        if (tasks.empty()) {
          lk.unlock();
          continue;
        }

        // else, goto wake state
        // and check `halt_flag`
      }

      // Atomic assign local variables
      task = tasks.front();
      num_acc_tasks++;
      tasks.pop();

      lk.unlock();

      // Run the task and increse `num_fin_tasks`
      assert(task.runnable != nullptr);
      task.runnable->runTask(task.task_id, task.num_total_tasks);
      num_fin_tasks++;
    }
  };

  // Thread pool setup here
  for (int i = 0; i < num_threads; ++i) {
    workers.emplace_back(worker, i);
  }

  m_task_sets.push_back(
      TaskSet{.runnable = nullptr, .num_total_tasks = 0, .task_set_id = 0});
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
  halt_flag = true;
  wake.notify_all();  // into wake state, check halt_flag
  for (int i = 0; i < num_threads; ++i) {
    workers[i].join();
  }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable,
                                               int        num_total_tasks) {
  assert(num_fin_tasks == 0);
  operate_queue.lock();
  for (int i = 0; i < num_total_tasks; ++i)
    tasks.push(Task{.runnable        = runnable,
                    .num_total_tasks = num_total_tasks,
                    .task_id         = i});
  operate_queue.unlock();

  while (num_fin_tasks != num_total_tasks) {
    wake.notify_all();
  }

  num_fin_tasks = 0;
  num_acc_tasks = 0;
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(
    IRunnable* runnable, int num_total_tasks, const std::vector<TaskID>& deps) {
  m_n_task_sets++;
  m_task_sets.push_back(TaskSet{.runnable        = runnable,
                                .num_total_tasks = num_total_tasks,
                                .task_set_id     = m_n_task_sets});

  if (deps.size() != 0) {
    for (auto& i : deps) m_DAG.addEdge(static_cast<int>(i), m_n_task_sets);
  } else {
    m_DAG.addEdge(0, m_n_task_sets);
  }

  return m_n_task_sets;
}

void TaskSystemParallelThreadPoolSleeping::sync() {
  // Not a accurate implementation
  auto topo_result = m_DAG.topo();
  for (auto& i : topo_result) {
    if (i == 0) continue;

    auto task = m_task_sets[i];
    operate_queue.lock();
    for (int i = 0; i < task.num_total_tasks; ++i)
      tasks.push(Task{.runnable        = task.runnable,
                      .num_total_tasks = task.num_total_tasks,
                      .task_id         = i});
    operate_queue.unlock();

    while (num_fin_tasks != task.num_total_tasks) {
      wake.notify_all();
    }

    num_fin_tasks = 0;
    num_acc_tasks = 0;
  }

  return;
}
