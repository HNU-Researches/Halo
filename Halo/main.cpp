#include <inttypes.h>
#include <unistd.h>

#include <atomic>
#include <iostream>
#include <string>
#include <thread>
// #include "/home/nft/CodeZip/Halo-main/third/pmdk/src/include/libpmem.h"
#include "Halo.hpp"



#define NONVAR 1
using namespace std;
// using namespace HALO;
int n = 30;
int main(int argc, char *argv[]) {

  cout<<"-------------------------\n";
  Halo<size_t, size_t> halo(1024);
  cout<<"-------------------------\n";

  int *r = new int[n];

  Pair_t<size_t, size_t> *rp = new Pair_t<size_t, size_t>[n];
  Pair_t<size_t, size_t> *rp2 = new Pair_t<size_t, size_t>[n];

  for (size_t i = 0; i < n; i++) {

    Pair_t<size_t, size_t> p(i, i);
    if(! (halo.Insert(p, &r[i]))) printf("Insert Failed!\n");
  }

  halo.wait_all();
  cout << "inserted." << endl;
  // halo.load_factor();
  for (size_t i = 0; i < n; i++) {
    rp[i].set_key(i);

    halo.Get(&rp[i]);
  }
  halo.get_all();
  for (size_t i = 0; i < n; i++) {
    cout << *reinterpret_cast<size_t *>(rp[i].key()) << "-" << rp[i].value()
         << endl;
  }
  for (size_t i = 0; i < n; i++) {

    Pair_t<size_t, size_t> p(i, rp[i].value() + 1);

    halo.Update(p, &r[i]);
  }
  for (size_t i = 0; i < n; i++) {
    rp2[i].set_key(i);

    halo.Get(&rp2[i]);
  }
  halo.get_all();
  for (size_t i = 0; i < n; i++) {
    cout << *reinterpret_cast<size_t *>(rp2[i].key()) << "-" << rp2[i].value()
         << endl;
  }
  return 0;
}