// #define _GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sched.h>
#include <iostream>
#include <thread>

#define ARRAY_SIZE 5000000

// extract CPU cycles from rdtsc register
uint64_t rdtsc() {
  unsigned int lo, hi;
  __asm__ __volatile__("rdtsc": "=a"(lo), "=d"(hi));
  return ((uint64_t) hi << 32) | lo;
}

unsigned arr_a[ARRAY_SIZE];

inline void work_cached(void) {
  unsigned long scratch = 0;
  for (int k = 0; k < ARRAY_SIZE; k++) {
    scratch += arr_a[k];
  }
}

inline void no_op(void) {

}

void bind_to_core(pthread_t thread, int cpu) {
  cpu_set_t cpuset;
  CPU_ZERO(& cpuset);
  CPU_SET(cpu, &cpuset);
  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}
