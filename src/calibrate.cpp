#include "utils.hh"

int main(int argc, char *argv[])
{
    if(argc != 4) {
      printf("Invoke with 3 arguments, c, t, and should_cache \n");
    }

    // Tuneable: the number of empty loops that constitute a trial
    int NUM_CYCLES = atoi(argv[1]);
    // How many trials to run
    int NUM_TRIALS = atoi(argv[2]);

    // store time for each trial
    uint64_t *results = (uint64_t*) malloc((NUM_TRIALS + 1)*sizeof(uint64_t));

    // should run the cache version of the benchmark
    int should_cache = atoi(argv[3]);

    // If should_cache, use work_cached function; otherwise use no_op
    void (*work)() = should_cache ? &work_cached : &no_op;

    printf("Running %d trials with %d cycles each.\n", NUM_TRIALS, NUM_CYCLES);

    if(should_cache) {
       printf("Running with cache accesses.\n");
    }

    // Fork so that we can run perf stat in another thread
    int pid = getpid();
    int cpid = fork();

    if(cpid == 0)
    {
        // Run perf stat
        char buf[50];
        sprintf(buf, "perf stat -p %d   > stat.log 2>&1",pid);
        execl("/bin/sh", "sh", "-c", buf, NULL);
    }
    else
    {
        // set the child the leader of its process group
        setpgid(cpid, 0);

	      // wait for perf stat to be running
        sleep(2);

        // start time
        results[0] = rdtsc();
        // printf("start time %lu", rdtsc());

        // Run NUM_TRIALS trials
        for(int i = 1; i < NUM_TRIALS + 1; i++)
        {
            // one trial is an empty loop of num_cycles
            for(int j = 0; j < NUM_CYCLES; j++) {
              work();
            }

            // store resulting cpu time
            results[i] = rdtsc();
        }

        // stop perf stat by killing child process
        kill(-cpid, SIGINT);

        // Print to stdout. Useful for shell scripting
        for(int i = 1; i < NUM_TRIALS+1; i++) {
           printf("%lu\n", results[i] - results[i-1]);
        }
    }
}
