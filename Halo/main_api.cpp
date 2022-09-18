#include <inttypes.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include "cpucounters.h"
#include "timer.h"
#include "utils.h"
#include "hash_api.h"
// const size_t INVALID = UINT64_MAX;

#define PM_PCM
using namespace pcm;
int main(int argc, char *argv[]) {
  // long int a[8] =
  // { 8388608,
  //   16777216,
  //   33554432,
  //   67108864,
  //   134217728,
  //   268435456,
  //   536870912,
  //   1073741824 };
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


  long int a[8] =
  { 8388608,
    16777216,
    33554432,
    67108864,
    134217728,
    268435456,
    536870912,
    1073741824 };
 

    //hash_api **ht_p_r = (hash_api **)malloc((8 ) * sizeof(hash_api *));
    
    // for (int i = 0; i < 8 ; i++)
    // {
    //     string table_name = "halo_";
    //     table_name += to_string(i);
    //     cout<<table_name<<endl;
    //     ht_p_r[i] = new hash_api((a[i]*2) , table_name);
    //     printf("i:%d\n", i);
    // }
    int *r = new int[1024];

  for (int j = 0; j <1; j++) {
    long int n = a[j];

    hash_api *h = new hash_api(n * 2);
    // hash_api *h = ht_p_r[j];


    Pair_t<size_t, size_t> *rp = new Pair_t<size_t, size_t>[n];
    // Pair_t<size_t, size_t> *insert = new Pair_t<size_t, size_t>[n];

    // for(long int i=0;i<n;i++){
    //     Pair_t<size_t, size_t> temp(i,i);
    //     rp[i] = temp;

    // }
    printf("------------start-------------\n");
    time_t start_time = 0, end_time = 0,mid_time =0;
    start_time = clock();

    for (long int i = 0; i < n; i++) {
          // printf("------------%d-------------\n",i);
      if (!(h->insert_halo(i, i, &r[i%1024])))
        ;
      // printf("Insert Failed!\n");
    }
    int match = 0;
    h->wait_insert();
    printf("------------mid-------------\n");
#ifdef PM_PCM
    auto before_state = getSystemCounterState();
#endif
    mid_time = clock();
    for (long int i = 0; i < n; i++) {
      if (h->find(i, &(rp[i]))){
          // printf("Find key %d success!,value:%d\n", *reinterpret_cast<size_t*>(rp[i].key()), rp[i].value());
          // match ++;
      }
        // // ;

      // else
      //     printf("Find key %d failed!\n", *reinterpret_cast<size_t*>(rp[i].key()));
    }

    h->wait_get();
    // for (size_t i = 0; i < n; i++)
    // {
    //     cout << *reinterpret_cast<size_t *>(rp[i].key()) << "-" <<
    //     rp[i].value()
    //          << endl;
    // }
    printf("------------end-------------\n");
    // printf("match:%d\n",match);


    end_time = clock();
  double t1 =  (double)(mid_time - start_time) / CLOCKS_PER_SEC;
  double t2 =  (double)(end_time - mid_time) / CLOCKS_PER_SEC;
#ifdef PM_PCM
    auto after_sstate = getSystemCounterState();
    cout << "MB WrittenToPMM: "
         << getBytesWrittenToPMM(before_state, after_sstate) / (1024 * 1024) << " "
         << (getBytesWrittenToPMM(before_state, after_sstate) / (1024 * 1024)) /
                (t2)
         << " MB/s" << endl;
    cout << "MB ReadFromPMM: "
         << getBytesReadFromPMM(before_state, after_sstate) / 1000000 << " "
         << (getBytesReadFromPMM(before_state, after_sstate) / 1000000.0) /
                (t1)
         << " MB/s" << endl;

#endif


    // printf("count:%ld\n",count);
    printf("size:%ld(MB)  ",n/65536);
    printf("Insert Time Used: %f Find Time Used: %f", (double)(mid_time - start_time) / CLOCKS_PER_SEC,  (double)(end_time - mid_time) / CLOCKS_PER_SEC);
    printf("Total Time Used: %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
  }

  return 0;
}