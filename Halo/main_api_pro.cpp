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
#include "hash_api.h"
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

  long int a[8] = {128,      16777216,  33554432,  67108864,
                   134217728, 268435456, 536870912, 1073741824};

  int *r = new int[1024];
  int num_pass_bit = 6;
  for (int j = 0; j < 1; j++) {
    long int n = a[j];

    hash_api *h = new hash_api(n, num_pass_bit);

    printf("------------start-------------\n");
    time_t start_time = 0, end_time = 0, mid_time = 0;
    start_time = clock();

    for (long int i = 0; i < n; i++) {
      // printf("------------%d-------------\n",i);
      if (!(h->insert_halo(i, i, &r[i % 1024])))
        ;
      // printf("Insert Failed!\n");
    }
    int match = 0;
    h->wait_insert();
    printf("------------mid-------------\n");
    mid_time = clock();
    Pair_t<size_t, size_t> *key_p = new Pair_t<size_t, size_t>[n];
    long int ll = 0;
    for (int k = 0; k < (1 << num_pass_bit); k++) {
      size_t size_partition = h->get_size_partition(k);
      printf("partition number:%d partition size:%d\n", k, size_partition);
      for (int l = 0; l < size_partition; l++) {
        // size_t key_get = (h->get_from_partition(k, l));
        printf("k:%d, l:%d, ll:%d\n",k, l,ll);
        (h->get_from_partition(k, l, &key_p[ll]));

        ll++;

      }
    }
    h->wait_get();

    printf("------------end-------------\n");
    // printf("match:%d\n", match);
    // size_t count = 0;

    for (int l = 0; l < n; l++) {
      size_t key_get = *reinterpret_cast<size_t *>(key_p[l].key());
      printf("l:%d  key:%ld\n", l, key_get);
    }


    end_time = clock();
    // printf("count:%ld\n",count);
    printf("size:%ld(MB)  ", n / 65536);
    printf("Insert Time Used: %f Find Time Used: %f",
           (double)(mid_time - start_time) / CLOCKS_PER_SEC,
           (double)(end_time - mid_time) / CLOCKS_PER_SEC);
    printf("Total Time Used: %f\n",
           (double)(end_time - start_time) / CLOCKS_PER_SEC);
  }

  return 0;
}