#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"

#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <cassert>
#include <condition_variable>

struct Task {
  IRunnable* runnable;
  int        num_total_tasks, task_id;
};

struct TaskSet {
  IRunnable* runnable;
  int        num_total_tasks, task_set_id;
};

class DAG {
 public:
  DAG()  = default;
  ~DAG() = default;
  void clear() { m_graph.clear(); }
  void addEdge(int u, int v) {
    const int size = m_graph.size();
    if (std::max(u, v) + 1 >= size) {
      m_graph.resize(std::max(u, v) + 1, std::vector<int>());
    }

    m_graph[u].push_back(v);
  }

  std::vector<int> topo() {
    std::vector<int> in_degree, ans;
    std::queue<int>  q;

    const int size = m_graph.size();
    in_degree.resize(size, 0);
    for (const auto& i : m_graph)
      for (const auto& j : i) in_degree[j]++;

    q.push(0);

    while (!q.empty()) {
      int u = q.front();
      q.pop();
      ans.push_back(u);
      for (auto& i : m_graph[u])
        if (--in_degree[i] == 0) q.push(i);
    }

    // assert(ans.size() == size);
    return ans;
  }

 protected:
  std::vector<std::vector<int>> m_graph;
};

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial : public ITaskSystem {
 public:
  TaskSystemSerial(int num_threads);
  ~TaskSystemSerial();
  const char* name();
  void        run(IRunnable* runnable, int num_total_tasks);
  TaskID      runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                               const std::vector<TaskID>& deps);
  void        sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn : public ITaskSystem {
 public:
  TaskSystemParallelSpawn(int num_threads);
  ~TaskSystemParallelSpawn();
  const char* name();
  void        run(IRunnable* runnable, int num_total_tasks);
  TaskID      runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                               const std::vector<TaskID>& deps);
  void        sync();

 protected:
  int num_threads;
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning : public ITaskSystem {
 public:
  TaskSystemParallelThreadPoolSpinning(int num_threads);
  ~TaskSystemParallelThreadPoolSpinning();
  const char* name();
  void        run(IRunnable* runnable, int num_total_tasks);
  TaskID      runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                               const std::vector<TaskID>& deps);
  void        sync();

 protected:
  int                      num_threads{0};
  std::vector<std::thread> workers{};
  std::queue<int>   tasks{};  // specifying the tasks waiting to be complete
  std::atomic<int>  num_acc_tasks{0}, num_fin_tasks{0};
  std::atomic<bool> halt_flag{false};
  std::mutex        launching_task{};

  IRunnable* global_runnable{nullptr};
  int        global_num_total_tasks{0};
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping : public ITaskSystem {
 public:
  TaskSystemParallelThreadPoolSleeping(int num_threads);
  ~TaskSystemParallelThreadPoolSleeping();
  const char* name();
  void        run(IRunnable* runnable, int num_total_tasks);
  TaskID      runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                               const std::vector<TaskID>& deps);
  void        sync();

 protected:
  int                      num_threads{0};
  std::vector<std::thread> workers{};
  std::queue<Task> tasks{};  // specifying the tasks waiting to be complete
  std::mutex       operate_queue{};
  std::condition_variable wake{};
  std::atomic<int>        num_acc_tasks{0}, num_fin_tasks{0};
  std::atomic<bool>       halt_flag{false};

  std::vector<TaskSet> m_task_sets{};
  int                  m_n_task_sets{0};
  DAG                  m_DAG{};
};

#endif
