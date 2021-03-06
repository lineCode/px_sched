// Example-6:
// Manually working with sync objects to control the launch of tasks

#define PX_SCHED_IMPLEMENTATION 1
#include "../px_sched.h"
#include "mem_check.h"

int main(int, char **) {
  atexit(mem_report);
  px::Scheduler schd;
  px::SchedulerParams s_params;
  s_params.mem_callbacks.alloc_fn = mem_check_alloc;
  s_params.mem_callbacks.free_fn = mem_check_free;
  schd.init(s_params);

  px::Sync s1,s2;
  for(size_t i = 0; i < 10; ++i) {
    auto job = [i] {
      printf("Phase 1: Task %zu completed from %s\n",
       i, px::Scheduler::current_thread_name());
    };
    schd.run(job, &s1);
  }

  // manually increment the sync object, to control any task that will be
  // attached to it. Tasks are executed when sync objects reach zero.
  schd.incrementSync(&s1);

  for(size_t i = 0; i < 10; ++i) {
    auto job = [i] {
      printf("Phase 2: Task %zu completed from %s\n",
       i, px::Scheduler::current_thread_name());
    };
    schd.runAfter(s1, job, &s2);
  }

  printf("Holding sync object 1 to prevent launch of phase2\n");
  std::this_thread::sleep_for(std::chrono::seconds(2));
  printf("releasing Sync-1...\n");
  schd.decrementSync(&s1);

  px::Sync last = s2;
  printf("Waiting for tasks to finish...\n");
  schd.waitFor(last); // wait for all tasks to finish
  printf("Waiting for tasks to finish...DONE \n");


  return 0;
}
