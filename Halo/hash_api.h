#ifndef _HASH_API_H
#define _HASH_API_H

#include "Halo.hpp"

const size_t pool_size = 1024ul * 1024ul * 1024ul * 64ul;
// using namespace HALO;
class hash_api {
 public:
  Halo<size_t, size_t> *t;
  hash_api(size_t sz);
  hash_api(size_t sz, size_t num_pass);
  hash_api(size_t sz, string table_name);


  ~hash_api() { delete t; };
  bool find(size_t key, void *p);
  bool insert(size_t key,  size_t value_len, char * value,  int tid = 0, int *r = nullptr);

  bool insert_halo(size_t key, size_t value, int *r = nullptr);
  bool insert_halo(size_t key, char * value, int *r = nullptr);

  bool insert_halo(size_t key, size_t value, int index, int *r = nullptr);
  void wait();
  void wait_get();
  void wait_insert();
  size_t get_key_index(size_t table_number, size_t *table_index, size_t *buckets_number,size_t *buckets_index);
  size_t get_table_size(size_t table_number);
  size_t get_size_partition(int pass_number);
  void get_from_partition(int pass_number,int curr_chain, Pair_t<size_t, size_t>  *p);
};

#endif
