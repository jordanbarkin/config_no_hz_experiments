#!/bin/zsh

# recompile
make all

# for distinguishing runs; change manually depending on environment
TRIAL_NUM=1
PREFIX="NO_HZ"
FILENAME_PREFIX="${PREFIX}_TRIAL_${TRIAL_NUM}_"
TARGET_DIR="data/${FILENAME_PREFIX}/"

mkdir -p $TARGET_DIR
FILENAME_SUFFIX=".txt"

# number of phases to run in all benchmarks
NUM_PHASES=100000

# Core range to peg for multithreaded
FIRST_CORE=4
LAST_CORE=47

# cycles for phase lengths
# precomputed manually in associative array
declare -A cycle_lengths
declare -A cycle_lengths_with_cache

cycle_lengths['1MICRO']=855
cycle_lengths['10MICRO']=8550
cycle_lengths['100MICRO']=85500
cycle_lengths['1MS']=855000

cycle_lengths_with_cache['1MICRO']=1
cycle_lengths_with_cache['10MICRO']=2
cycle_lengths_with_cache['100MICRO']=16
cycle_lengths_with_cache['1MS']=160

# single threaded benchmarks, no caching
for name cycles in ${(kv)cycle_lengths}
do
  # taskset -c $FIRST_CORE
  ./calibrate.out "${cycles}" $NUM_PHASES 0 > "${TARGET_DIR}SINGLE_THREADED_NO_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
  cp stat.log "${TARGET_DIR}stats_SINGLE_THREADED_NO_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
done

echo "Finished singlethreaded no cache"

# single threaded benchmarks, with caching
for name cycles in ${(kv)cycle_lengths_with_cache}
do
  ./calibrate.out "${cycles}" $NUM_PHASES 1 > "${TARGET_DIR}SINGLE_THREADED_WITH_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
  cp stat.log "${TARGET_DIR}stats_SINGLE_THREADED_WITH_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
done

echo "Finished singlethreaded with cache"

# multi threaded benchmarks, no caching
for name cycles in ${(kv)cycle_lengths}
do
  ./multithreaded.out "${cycles}" $NUM_PHASES 0 $FIRST_CORE $LAST_CORE > "${TARGET_DIR}MULTI_THREADED_NO_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
  cp stat.log "${TARGET_DIR}stats_MULTI_THREADED_NO_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
done

echo "Finished multithreaded no cache"

# multi threaded benchmarks, with caching
for name cycles in ${(kv)cycle_lengths_with_cache}
do
  ./multithreaded.out "${cycles}" $NUM_PHASES 1 $FIRST_CORE $LAST_CORE > "${TARGET_DIR}MULTI_THREADED_WITH_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
  cp stat.log "${TARGET_DIR}stats_MULTI_THREADED_WITH_CACHE_${FILENAME_PREFIX}${name}${FILENAME_SUFFIX}"
done

echo "Finished multithreaded with cache"

echo "Done"
