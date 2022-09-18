#include <inttypes.h>
#include <sys/time.h> /* gettimeofday */
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include "cpucounters.h"
#include "hash_api.h"
#include "timer.h"
#include "utils.h"
// const size_t INVALID = UINT64_MAX;

#include <pthread.h>
#include <sched.h>
static int node_mapping[] = {0,  2,  4,  6,  8,  10, 12, 14, 16, 18, 20,
                             22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42,
                             44, 46, 48, 50, 52, 54, 56, 58, 60, 62};
#define PM_PCM
#define CACHE_LINE_SIZE 64
using namespace pcm;

#define BARRIER_ARRIVE(B, RV)                           \
  RV = pthread_barrier_wait(B);                         \
  if (RV != 0 && RV != PTHREAD_BARRIER_SERIAL_THREAD) { \
    printf("Couldn't wait on barrier\n");               \
    exit(EXIT_FAILURE);                                 \
  }

typedef struct arg_t arg_t;

struct arg_t {
  size_t start;
  size_t end;
  size_t lenght;
  size_t *key;
  size_t *value;
  hash_api *h;
  int *r_insert;
  Pair_t<size_t, size_t> *rp;
  bool is_insert;

  struct timeval start_insert, end_insert, start_read, end_read;
  int tid;
  pthread_barrier_t *barrier;

} __attribute__((aligned(CACHE_LINE_SIZE)));

void *run(void *arg);

int main(int argc, char *argv[]) {

  // string workload = argv[1];
  // string load_data = "";
  // string run_data = "";
  // if (workload.find("ycsb") != string::npos) {
  //   load_data = "../YCSB/workloads/ycsb_load_workload";
  //   load_data += workload[workload.size() - 1];
  //   run_data = "../YCSB/workloads/ycsb_run_workload";
  //   run_data += workload[workload.size() - 1];
  //   wlt = YCSB;

  long int a[8] = {8388608,   16777216,  33554432,  67108864,
                   134217728, 268435456, 536870912, 1073741824};

  for (int j = 4; j < 5; j++) {
    long int n = a[j];
    int num_thread = atoi(argv[2]);
    pthread_t pth[num_thread];
    arg_t args[num_thread];

    hash_api *h = new hash_api(n * 2);
    Pair_t<size_t, size_t> *rp = new Pair_t<size_t, size_t>[n];
#ifdef PM_PCM
    set_signal_handlers();
    PCM *m = PCM::getInstance();
    auto status = m->program();
    if (status != PCM::Success) {
      std::cout << "Error opening PCM: " << status << std::endl;
      if (status == PCM::PMUBusy)
        m->resetPMU();
      else
        exit(0);
    }
    print_cpu_details();
#endif

#ifdef PM_PCM
    auto before_state = getSystemCounterState();
#endif

    size_t *key_t = (size_t *)calloc(n, sizeof(size_t));
    size_t *val_t = (size_t *)calloc(n, sizeof(size_t));
    for (size_t k = 0; k < n; k++) {
      key_t[k] = k;
      val_t[k] = k;
    }

    pthread_barrier_t barrier;
    int rv;
    rv = pthread_barrier_init(&barrier, NULL, num_thread);
    if (rv != 0) {
      printf("[ERROR] Couldn't create the barrier\n");
      exit(EXIT_FAILURE);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    size_t start = 0;
    cpu_set_t set;

    for (int i = 0; i < num_thread; i++) {
      // int cpu_idx = node_mapping[i];

      // CPU_ZERO(&set);
      // CPU_SET(cpu_idx, &set);
      // pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

      int *r = new int[1024];
      args[i].lenght = n / num_thread;
      args[i].start = start;
      args[i].end = args[i].start + args[i].lenght;

      args[i].rp = rp;
      args[i].h = h;
      args[i].r_insert = r;

      args[i].key = key_t;
      args[i].value = val_t;
      args[i].is_insert = true;
      args[i].barrier = &barrier;
      args[i].tid = i;

      pthread_create(&pth[i], &attr, run, (void *)&args[i]);
    }

    for (int i = 0; i < num_thread; i++) {
      pthread_join(pth[i], NULL);
    }

    double time_insert, time_read;
    time_insert = (((args[0].end_insert).tv_sec * 1000000L +
                    (args[0].end_insert).tv_usec) -
                   ((args[0].start_insert).tv_sec * 1000000L +
                    (args[0].start_insert).tv_usec));
    time_read =
        (((args[0].end_read).tv_sec * 1000000L + (args[0].end_read).tv_usec) -
         ((args[0].start_read).tv_sec * 1000000L +
          (args[0].start_read).tv_usec));

    time_insert /= 1000000.0;
    time_read /= 1000000.0;
    printf("Data size:%ld(MB)  \n", n / 65536);
    printf("Write time :       Read time:   (s)\n");
    printf("  %.4lf       %.4lf                \n", time_insert, time_read);

#ifdef PM_PCM
    auto after_sstate = getSystemCounterState();
    cout << "MB WrittenToPMM: "
         << getBytesWrittenToPMM(before_state, after_sstate) / (1024 * 1024)
         << "(MB) "
         << (getBytesWrittenToPMM(before_state, after_sstate) / (1024 * 1024)) /
                (time_insert)
         << " MB/s" << endl;
    cout << "MB ReadFromPMM: "
         << getBytesReadFromPMM(before_state, after_sstate) / (1024 * 1024)
         << "(MB) "
         << (getBytesReadFromPMM(before_state, after_sstate) / (1024 * 1024)) /
                (time_read)
         << " MB/s" << endl;

#endif
  }

  return 0;
}

void *run(void *param) {
  // INSERT

  arg_t *args = (arg_t *)param;
  int rv;
  int my_tid = args->tid;
  int *r = args->r_insert;
  Pair_t<size_t, size_t> *rp = args->rp;
  hash_api *h = args->h;
  size_t *key_t = args->key;
  size_t *val_t = args->value;

  BARRIER_ARRIVE(args->barrier, rv);

  if (my_tid == 0) {
    gettimeofday(&args->start_insert, NULL);
  }
  // printf("args->start:%ld  args->end:%ld \n", args->start,  args->end);
  for (size_t i = args->start; i < args->end; i++) {
    //  printf("i%ld key_t[i]:%ld val_t[i]:%ld\n",i,key_t[i], val_t[i]);
    h->insert_halo(key_t[i], val_t[i], &r[i % 1024]);
    // printf("Insert Failed!\n");
  }

  h->wait_insert();

  BARRIER_ARRIVE(args->barrier, rv);

  if (my_tid == 0) {
    gettimeofday(&args->end_insert, NULL);
    gettimeofday(&args->start_read, NULL);
  }

  // READ

  BARRIER_ARRIVE(args->barrier, rv);

  for (size_t i = args->start; i < args->end; i++) {
    h->find(key_t[i], &(rp[i]));
  }
  h->wait_get();

  BARRIER_ARRIVE(args->barrier, rv);
  if (my_tid == 0) {
    gettimeofday(&args->end_read, NULL);
  }

  size_t matches = 0;
  for (size_t i = args->start; i < args->end; i++) {
    if ((*reinterpret_cast<size_t *>(rp[i].key()) == key_t[i])) matches++;
  }
  if (matches != args->lenght) printf("Find FAULT!\n");

  return ((void *)0);
}
