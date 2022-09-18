#include "hash_api.h"
#include <string>
// using namespace HALO;

hash_api::hash_api(size_t sz) {
  PM_PATH = "/mnt/pmem/hash/HaLo/";
  t = new Halo<size_t, size_t>(sz);
}

hash_api::hash_api(size_t sz, size_t num_pass) {
  PM_PATH = "/mnt/pmem/hash/HaLo/";
  t = new Halo<size_t, size_t>(sz, num_pass);
}

bool hash_api::find(size_t key, void *p) {
  auto pt = reinterpret_cast<Pair_t<size_t, size_t> *>(p);
  pt->set_key(key);
  return t->Get(pt);
}


bool hash_api::insert(size_t key,  size_t value_len, char * value,  int tid = 0, int *r = nullptr) {
  // printf("insert!!!!!!!\n");
  Pair_t<size_t, size_t> p(key,*reinterpret_cast<size_t *> (value));
  return t->Insert(p, r);
  return true;
}


bool hash_api::insert_halo(size_t key,  char * value,  int *r = nullptr) {
  // printf("insert!!!!!!!\n");
  Pair_t<size_t, size_t> p(key,*reinterpret_cast<size_t *> (value));
  return t->Insert(p, r);
  return true;
}

bool hash_api::insert_halo(size_t key, size_t value, int *r = nullptr) {
  // printf("insert!!!!!!!\n");
  Pair_t<size_t, size_t> p(key, value);
  return t->Insert(p, r);
  return true;
}

void hash_api::wait() { t->wait_all(); }
void hash_api::wait_insert() { t->wait_all(); }
void hash_api::wait_get() { 
  if(t->Is_pro())  t->Gets_pro();
  else t->get_all(); 
}

size_t hash_api::get_key_index(size_t table_number, size_t *table_index,size_t *buckets_number, size_t *buckets_index){
  Pair_t<size_t, size_t> *rp = new Pair_t<size_t, size_t>[1];
  t->Get_key(table_number,table_index,buckets_number, buckets_index,&rp[0]);
  size_t the_key = *reinterpret_cast<size_t*>(rp[0].key());     
  return the_key;
}

size_t hash_api::get_table_size(size_t table_number){
  return t->Get_table_size(table_number);
}

size_t hash_api::get_size_partition(int pass_number) {
  return t->get_partition(pass_number);
}

void hash_api::get_from_partition(int pass_number,int curr_chain, Pair_t<size_t, size_t>  *p) {
  t->Get(pass_number,curr_chain, p);      //0.13
  // size_t tmp_key = t->Get(pass_number,curr_chain, p);
  // return tmp_key;
  // return 1;
}