// My linter made the spacing weird, but this is due soon so it stays.

#include "utils.hh"
#include <atomic>
#include <pthread.h>

std::atomic<int> *barriers;
int num_cores = 0;
void(*work)();
uint64_t *results;

inline void reach_barrier(int phase) {
  barriers[phase]++;
}

inline void await_barrier(int phase) {
  int barrier_passed = false;
  while (barriers[phase] < num_cores) {
    // printf("Waiting at barrier %d which is at %d\n",phase, barriers[phase].load());
    // spin until phase is done
  }
}

struct worker_args {
  int c;
  int t;
  int cpu_num;
  bool leader;

  worker_args(int c, int t, int cpu_num, bool leader) {
    this->c = c;
    this->t = t;
    this->cpu_num = cpu_num;
    this->leader = leader;
  }
};

void * worker(void * args) {
  worker_args * a = (worker_args * ) args;
  // bind each worker to its own core, based on its cpu num
  for (int i = 0; i <= a -> t; i++) {
    // store cpu time if leader
    if (a -> leader) {
      results[i] = rdtsc();
    }

    // one phase is an loop of num_cycles
    for (int j = 0; j < a -> c; j++) {
      work();
    }

    // printf("At phase %d\n", i);
    reach_barrier(i);
    await_barrier(i);
  }
  return nullptr;
}

int main(int argc, char * argv[]) {
  if (argc != 6) {
    printf("Invoke with 5 arguments, c, t, should_cache, first cpu in range, and last cpu in range\n");
  }
  // Fork so that we can run perf stat in another thread
  int pid = getpid();
  int cpid = fork();

  if (cpid == 0) {
    // Run perf stat
    char buf[50];
    sprintf(buf, "perf stat -p %d   > stat.log 2>&1", pid);
    execl("/bin/sh", "sh", "-c", buf, NULL);
  } else {
    // set the child the leader of its process group
    setpgid(cpid, 0);

    // wait for perf stat to be running
    sleep(2);
    printf("Perf stat started.\n");

    // Tuneable: the number of empty loops that constitute a trial
    int NUM_CYCLES = atoi(argv[1]);

    // How many trials to run
    int NUM_PHASES = atoi(argv[2]);

    // store time for each trial
    results = (uint64_t*) calloc((NUM_PHASES + 100),sizeof(uint64_t));

    // should run the cache version of the benchmark
    int should_cache = atoi(argv[3]);
    // If should_cache, use work_cached function; otherwise use no_op
    work = should_cache ? &work_cached : &no_op;

    int first_cpu = atoi(argv[4]);
    int last_cpu = atoi(argv[5]);
    num_cores = last_cpu - first_cpu + 1;

    // barriers, initialized to 0
    barriers = (std::atomic <int>* ) malloc(sizeof(std::atomic <int> ) * (2 + NUM_PHASES));
    for (int x = 0; x < NUM_PHASES; x++) {
      barriers[x] = 0;
    }

    printf("Running %d trials with %d cycles each.\n", NUM_PHASES, NUM_CYCLES);

    if (should_cache) {
      printf("Running with cache accesses.\n");
    }

    pthread_t tids[num_cores];
    printf("Launching threads...\n");

    // run all threads
    for (int t = first_cpu; t <= last_cpu; t++) {
      worker_args * a = new worker_args(NUM_CYCLES, NUM_PHASES, t, t == first_cpu); // only first core is leader

      pthread_create(&tids[t - first_cpu], NULL, & worker, a);
      bind_to_core(tids[t - first_cpu], t);
      printf("Started worker on core %d, index %d\n", t, (t - first_cpu));
    }

    printf("Start time: \n");
    printf("%lu\n", rdtsc());

    // await all threads.
    // printf("Waiting for threads\n");
    for (int t = 0; t < num_cores; t++) {
      pthread_join(tids[t], NULL);
    }

    printf("End time: \n");
    printf("%lu\n", rdtsc());

    printf("All threads finished\n");

    // stop perf stat by killing child process
    kill(-cpid, SIGINT);
    // printf("Perf stats killed\n");

    printf("Phase lengths:\n");
    // Print to stdout. Useful for shell scripting
    for (int i = 2; i < NUM_PHASES; i++) {
       printf("%lu\n", results[i] - results[i - 1]);
    }
  }
}
