# CXX := g++ -O0 -ggdb
# CFLAGS := -std=c++17 -march=native -L./ -mavx 

# all: Halo

# Halo: main.cpp libHalo.a
# 	$(CXX) $(CFLAGS) -o $@ $< -lHalo -lpthread -lpmem
# libHalo.a: Halo.cpp Halo.hpp 
# 	$(CXX) $(CFLAGS) -c -o libHalo.o $<
# 	ar rv libHalo.a libHalo.o
# clean:
# 	rm -f *.o *.a Halo

CXX = g++
CFLAGS = -std=c++17 -O3 -march=native -L./ 

PCM := -I ../pcm -L ../pcm

CFLAGS_PMDK := -std=c++17 -O3 -I./ -L./
tar = HALO # CCEH DASH CLEVEL PCLHT VIPER SOFT CLHT
all: $(tar)



libHalo.a: Halo.cpp Halo.hpp Pair_t.h
	g++ -std=c++17 -O3 -march=native -L./ -I./Halo -c -o libHalo.o $<
	ar rv libHalo.a libHalo.o

libHaloapi.a:hash_api.cpp  Halo.cpp 
	g++ -std=c++17 -O3 -fpermissive -march=native -L./ -c -o libHaloapi.o $<
	ar rv libHaloapi.a libHaloapi.o


HALO: main.cpp libHalo.a 
	$(CXX) -DHALOT -fpermissive $(CFLAGS) $(PCM) -o $@ $< -lHalo -pthread -mavx -lpmem
#	$(CXX) -DHALOT $(CFLAGS) $(PCM) -o $@ $< -lHalo -pthread -mavx   -L  /home/nft/CodeZip/Halo-main/third/pmdk/src/nondebug/libpmem.so

HALOapi: 
	$(CXX) -fpermissive  $(CFLAGS) $(PCM) main_api.cpp hash_api.cpp  Halo.cpp  -o HALOapi -pthread -mavx  -lpmem -lPCM 
#	$(CXX) -fpermissive  $(CFLAGS) main_api.cpp hash_api.cpp  Halo.cpp    -o HALOapi -pthread -mavx -L  /home/nft/CodeZip/Halo-main/third/pmdk/src/nondebug/libpmem.so

HALOapiPro: 
	$(CXX) -fpermissive  $(CFLAGS) $(PCM) main_api_pro.cpp hash_api.cpp  Halo.cpp  -o HALOapiPro -pthread -mavx -lpmem -lPCM 
#	$(CXX) -fpermissive  $(CFLAGS) main_api.cpp hash_api.cpp  Halo.cpp    -o HALOapi -pthread -mavx -L  /home/nft/CodeZip/Halo-main/third/pmdk/src/nondebug/libpmem.so

HALOapimt: 
	$(CXX) -fpermissive  $(CFLAGS) $(PCM) main_api_mt.cpp hash_api.cpp  Halo.cpp  -o HALOapimt -pthread -mavx  -lpmem -lPCM 
#	$(CXX) -fpermissive  $(CFLAGS) main_api.cpp hash_api.cpp  Halo.cpp    -o HALOapi -pthread -mavx -L  /home/nft/CodeZip/Halo-main/third/pmdk/src/nondebug/libpmem.so

HALOycsb: 
	$(CXX) -fpermissive  $(CFLAGS) $(PCM) main_ycsb.cpp hash_api.cpp  Halo.cpp  -o HALOycsb -pthread -mavx  -lpmem -lPCM 
#	$(CXX) -fpermissive  $(CFLAGS) main_api.cpp hash_api.cpp  Halo.cpp    -o HALOapi -pthread -mavx -L  /home/nft/CodeZip/Halo-main/third/pmdk/src/nondebug/libpmem.so


libHaloapi.so: hash_api.cpp  hash_api.h Halo.cpp  Pair_t.h
	$(CXX) hash_api.cpp  Halo.cpp  -fPIC -shared  -fpermissive $(CFLAGS)  -o libHaloapi.so -pthread -lpmem 
#	$(CXX) hash_api.cpp  Halo.cpp  -fPIC -shared  -fpermissive $(CFLAGS)  -o libHaloapi.so -pthread -L  /home/nft/CodeZip/Halo-main/third/pmdk/src/nondebug/libpmem.so

#export LD_LIBRARY_PATH=/home/nft/CodeZip/Halo-main/third/pmdk/src/nondebug
clean:
	rm -f *.o *.a *.so HALOapi HALO HALOapiPro HALOapimt HALOycsb 
