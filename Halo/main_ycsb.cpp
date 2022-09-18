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

  char values[] =
      "NvhE8N7yR26f4bbpMJnUKgHncH6QbsI10HyxlvYHKFiMk5nPNDbueF2xKLzteSd0NazU2APk"
      "JWXvBW2oUu8dkZnWMMu37G8TH2qm"
      "S0c8A9z41pxrC6ZU79OnfCZ06DsNXWY3U4dt1JTGQVvylBdZSlHWXC4PokCxsMdjv8xRptFM"
      "MQyHZRqMhNDnrsGKA12DEr7Zur0n"
      "tZpsyreMmPwuw7WMRnoN5wAYWtkqDwXyQlYb4RgtSc4xsonpTx2UhIUi15oJTx1CXAhmECac"
      "CQfntFnrSZt5qs1L64eeJ9Utus0N"
      "mKgEFV8qYDsNtJ21TkjCyCDhVIATkEugCw1BfNIB9AZDGiqXc0llp4rlJPl4bIG2QC4La3M1"
      "oh3yGlZTmdvN5pj1sIGkolpdoYVJ"
      "0NZM9KAo1d5sGFv9yGC7X0CTDOqyRu5c4NPktU70NbKqWNXa1kcaigIfeAuvJBs0Wso2osHz"
      "OjrbawgpfBPs1ePaWHgw7vbOcu9v"
      "Cqz1GnmdQw4mGSo4cc6tebQuKqLkQHuXa1MdRmzinBRoGQBQehqrDmmfNhcxfozcU7hOTjFA"
      "jryJ4HdSK57gOlrte5sZlvDW9rFd"
      "4OxG6WtFdZomRQPTNc4D9t7smqBR9EYDSjiAAqmIgZUiycHrlv6JQzEiexjqfGUbo8oJV6wi"
      "u7l3Jlfb94uByDxoexkMT5AjJzls"
      "er1dc9EfQz88q5Hv00g53Q3H6jcgicoY8YW5K4josd2e53ikesQi2kzqvTI9xxM5wtFexkFm"
      "8wFdMs6YmNpvNgTf37Hz204wX1Sf"
      "djFmCYEcP533LYcGB7CslEVMPYRZXHBT98XKtt8RqES7HBW65xSJRSj3qhIDUsgeu2Flo4Yq"
      "S68QoE69JzyBnwmmYw6uulVLVIAe"
      "iLl49oUhEiEjem8RrHPpEvrUoLDWwMdh14MfxwmEQbtGnUHEpRktUB6b7JTJN8OHBlLrvr71"
      "TkRK728ZgRv32rMZJ46O17qHTYc4"
      "AepNCGbpTII0J05OYiush6hiDo6H5pVHVUWy3nm7BBrBzEHVOCBMHNniw4CIzfavGLaUfgjl"
      "Bg0D4JBmYmkg0A4maCXsE9QTnGbA"
      "fQErGZkdMnRxXJ5EJ627e7zuFuVtazb0L65B3nU5R9tyUl2bTZiDcakK9evrTXoTkbkGjkCO"
      "iMSThGFScb6Lsgvl5wNCzlUZCxof"
      "jYQCLusRkXEm0CNVuifTnytctwLfKjwob4hJ0WxlQN9FV9Mm9zT61EQ8zEMrqr6hf7XMqhcQ"
      "R7DWAaf1fM4oNLIA7ZdKaspUaU6h"
      "oP2w3t3MktVaBp6MgS6Apbkb7EsihETHHqKFkKMCkYBbKfgsq7Jy49T1Wx2UJsD3XX03kVBb"
      "qRWmryYoMIqiCTCTqa0jIKzqQEnN";

uint64_t LOAD_SIZE;
uint64_t RUN_SIZE;
uint64_t MAX_SIZE_LOAD = 200000000ULL;
uint64_t MAX_SIZE_RUN = 200000000ULL;

typedef struct arg_t arg_t;

struct arg_t {
  size_t start;
  size_t end;
  size_t lenght;
  size_t *key;
  size_t *value;
  size_t *key_read;
  hash_api *h;
  int *r_insert;
  Pair_t<size_t, size_t> *rp;
  bool is_insert;

  struct timeval start_insert, end_insert, start_read, end_read;
  int tid;
  pthread_barrier_t *barrier;

} __attribute__((aligned(CACHE_LINE_SIZE)));
enum { OP_INSERT, OP_READ, OP_DELETE, OP_UPDATE };
void *run(void *arg);

int main(int argc, char *argv[]) {

  string workload = argv[1];
  string load_data = "";
  string run_data = "";
  if (workload.find("ycsb") != string::npos) {
    load_data = "../YCSB/workloads/ycsb_load_workload";
    load_data += workload[workload.size() - 1];
    run_data = "../YCSB/workloads/ycsb_run_workload";
    run_data += workload[workload.size() - 1];
  }

  if (workload == "ycsbi") MAX_SIZE_RUN = 800000000;
  string insert("INSERT");
  string read("READ");
  ifstream infile_load(load_data);
  string op;
  uint64_t key;
  uint64_t value_len;
  vector<uint64_t> init_keys(MAX_SIZE_LOAD);
  vector<uint64_t> keys(MAX_SIZE_RUN);
  vector<uint64_t> init_value_lens(MAX_SIZE_LOAD);
  vector<uint64_t> value_lens(MAX_SIZE_RUN);
  vector<int> ops(MAX_SIZE_RUN);
  int count = 0;
  while ((count < MAX_SIZE_LOAD) && infile_load.good()) {
    infile_load >> op >> key >> value_len;
    if (!op.size()) continue;
    if (op.size() && op.compare(insert) != 0) {
      cout << "READING LOAD FILE FAIL!\n";
      cout << op << endl;
      return;
    }
    init_keys[count] = key;
    init_value_lens[count] = value_len;
    count++;
  }
  LOAD_SIZE = count;
  infile_load.close();
  fprintf(stderr, "Loaded %lu keys for initialing.\n", LOAD_SIZE);

  int *r = new int[1024];
  ifstream infile_run(run_data);
  count = 0;
  printf("MAX_SIZE_RUN:%ld\n", MAX_SIZE_RUN);
  while ((count < MAX_SIZE_RUN) && infile_run.good()) {
    infile_run >> op >> key;
    if (op.compare(insert) == 0) {
      infile_run >> value_len;
      ops[count] = OP_INSERT;
      keys[count] = key;
      value_lens[count] = value_len;
    } else if (op.compare(read) == 0) {
      if (workload == "ycsbi")
        ops[count] = OP_DELETE;
      else
        ops[count] = OP_READ;
      keys[count] = key;
    } else {
      continue;
    }
    count++;
  }
  RUN_SIZE = count;
  fprintf(stderr, "Loaded %d keys for running.\n", count);

  long int n = LOAD_SIZE;
  int num_thread = atoi(argv[2]);
  pthread_t pth[num_thread];
  arg_t args[num_thread];
  auto part = LOAD_SIZE / num_thread;

  hash_api *h = new hash_api(16 * 1024 * 1024);
  // hash_api *h = new hash_api(LOAD_SIZE * 2);

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

    size_t *key_r = (size_t *)calloc(n, sizeof(size_t));
    for (size_t k = 0; k < n; k++) {
      key_t[k] = init_keys[k];
      val_t[k] = k;

      key_r[k] = keys[k];
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

      // int *r = new int[1024];
      args[i].lenght = part;
      args[i].start = start;
      args[i].end = args[i].start + args[i].lenght;

      args[i].rp = rp;
      args[i].h = h;
      args[i].r_insert = r;

      args[i].key = key_t;
      args[i].value = val_t;

      args[i].key_read = key_r;

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
  size_t *key_r = args->key_read;


  BARRIER_ARRIVE(args->barrier, rv);

  if (my_tid == 0) {
    gettimeofday(&args->start_insert, NULL);
  }
  // printf("args->start:%ld  args->end:%ld \n", args->start,  args->end);
  for (size_t i = args->start; i < args->end; i++) {
    //  printf("i%ld key_t[i]:%ld val_t[i]:%ld\n",i,key_t[i], val_t[i]);
    h->insert_halo(key_t[i], reinterpret_cast<char *>(values), &r[i % 1024]);
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
    h->find(key_r[i], &(rp[i]));
  }
  h->wait_get();

  BARRIER_ARRIVE(args->barrier, rv);
  if (my_tid == 0) {
    gettimeofday(&args->end_read, NULL);
  }

  // size_t matches = 0;
  // for (size_t i = args->start; i < args->end; i++) {
  //   if ((*reinterpret_cast<size_t *>(rp[i].value()) != 0)) matches++;
  // }
  // if (matches != args->lenght) printf("Find FAULT!\n");

  return ((void *)0);
}
