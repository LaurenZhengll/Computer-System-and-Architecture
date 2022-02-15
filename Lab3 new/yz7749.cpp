/*
   Cache Simulator
   Level one L1 and level two L2 cache parameters are read from file
   (block size, line per set and set per cache).
   The 32 bit address is divided into:
   -tag bits (t)
   -set index bits (s)
   -block offset bits (b)

   s = log2(#sets)   b = log2(block size)  t=32-s-b
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>


using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss


struct config{
  int L1blocksize;
  int L1setsize;
  int L1size;
  int L2blocksize;
  int L2setsize;
  int L2size;
};

// you can define the cache class here, or design your own data structure for L1 and L2 cache
class cache {
  //config configParams;
  //vector<vector<unsigned long> > myCache;

  private:
    unsigned long setIndexBit, blockOffset, tagBit, indexSize;
    vector<vector<unsigned long> > myCache;
    vector<vector<bool> > myValidBit;

  public:
     //cache constructor
    cache(unsigned long blockSize, unsigned long setSize, unsigned long cacheSize){   
      blockOffset = (unsigned long)log2(blockSize); // offset bits: b = log2(block size) 
      setIndexBit = (unsigned long)log2((cacheSize) * (pow(2,10)) / (blockSize*setSize)); // index bits: s = log2(#sets)
      tagBit = 32 - this->setIndexBit - this->blockOffset; //tag bits: t=32-s-b
      indexSize = (unsigned long)pow(2, setIndexBit); // number of index
      //initialize cache array and valid bit array
      myCache.resize(setSize);
      myValidBit.resize(setSize);
      for(unsigned long i = 0; i < setSize; i++) {
          myCache[i].resize(indexSize);
          myValidBit[i].resize(indexSize);
      }
    }

    //store bits of tag index and offset into index array
    vector<unsigned long> getIndex(){  
      vector<unsigned long> index(3, 0);
      index[0] = tagBit;
      index[1] = setIndexBit;
      index[2] = blockOffset;
      return index;
    }

    vector<vector<unsigned long>> &getCache(){  //get tag array
      return myCache;
    }

    vector<vector<bool>> &getValidBit(){  //get valid bit array
      return myValidBit;
    }

    unsigned long getIndexSize(){
      return indexSize;
    }
};      

int main(int argc, char* argv[]){
  config cacheconfig;
  ifstream cache_params;
  string dummyLine;
  cache_params.open(argv[1]);
  while(!cache_params.eof())  // read config file
  {
    cache_params>>dummyLine;
    cache_params>>cacheconfig.L1blocksize;
    cache_params>>cacheconfig.L1setsize;              
    cache_params>>cacheconfig.L1size;
    cache_params>>dummyLine;              
    cache_params>>cacheconfig.L2blocksize;           
    cache_params>>cacheconfig.L2setsize;        
    cache_params>>cacheconfig.L2size;
  }

  // Implement by you: 
  // initialize the hirearch cache system with those configs
  // probably you may define a Cache class for L1 and L2, or any data structure you like

  if (cacheconfig.L1setsize == 0) {  // full associative
      cacheconfig.L1setsize = cacheconfig.L1size * pow(2, 10) / cacheconfig.L1blocksize;
  }
  if (cacheconfig.L2setsize == 0) {
      cacheconfig.L2setsize = cacheconfig.L2size * pow(2, 10) / cacheconfig.L2blocksize;
  }

  //create cacheL1 and cacheL2 with constructor
  cache cacheL1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size);
  cache cacheL2(cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);

  //get bits of tag index and offset
  vector<unsigned long> IndexL1 = cacheL1.getIndex();
  vector<unsigned long> IndexL2 = cacheL2.getIndex();

  //set up counter
  vector<unsigned long> counterL1, counterL2;
  counterL1.resize(cacheL1.getIndexSize());
  counterL2.resize(cacheL2.getIndexSize());


  int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
  int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

  ifstream traces;
  ofstream tracesout;
  string outname;
  outname = string(argv[2]) + ".out";
  traces.open(argv[2]);
  tracesout.open(outname.c_str());

  string line;
  string accesstype;  // the Read/Write access type from the memory trace;
  string xaddr;       // the address from the memory trace store in hex;
  unsigned int addr;  // the address from the memory trace store in unsigned int;        
  bitset<32> accessaddr; // the address from the memory trace store in the bitset;

  if (traces.is_open()&&tracesout.is_open()){    
    while (getline (traces,line)){   // read mem access file and access Cache
      istringstream iss(line); 
      if (!(iss >> accesstype >> xaddr)) {break;}
      stringstream saddr(xaddr);
      saddr >> std::hex >> addr;
      accessaddr = bitset<32> (addr);

      // get tag and index by converting bit to unsigned long of an address
      long tagL1, indexL1, tagL2, indexL2;
      tagL1 = bitset<32>(accessaddr.to_string().substr(0, IndexL1[0])).to_ulong();
      indexL1 = bitset<32>(accessaddr.to_string().substr(IndexL1[0], IndexL1[1])).to_ulong();
      tagL2 = bitset<32>(accessaddr.to_string().substr(0, IndexL2[0])).to_ulong();
      indexL2 = bitset<32>(accessaddr.to_string().substr(IndexL2[0], IndexL2[1])).to_ulong();

      // tag matching in L1
      bool hitL1 = false, hitL2 = false;
      for (int i = 0; i < cacheconfig.L1setsize; i++){
        if(cacheL1.getCache()[i][indexL1] == tagL1 && cacheL1.getValidBit()[i][indexL1]){
          hitL1 = true; //hit
          break;
        }
        else{        
          hitL1 = false;  //miss
        }
      }
      
      // if not matched in L1, tag matching in L2
       if(!hitL1){
        for (int i = 0; i < cacheconfig.L2setsize; i++){
          if(cacheL2.getCache()[i][indexL2] == tagL2 && cacheL2.getValidBit()[i][indexL2]){
            hitL2 = true; //hit
            break;
          }
          else{        
            hitL2 = false;  //miss
          }
        }
      }

      // access the L1 and L2 Cache according to the trace;
      if (accesstype.compare("R")==0){    
        //Implement by you:
        // read access to the L1 Cache, 
        //  and then L2 (if required), 
        //  update the L1 and L2 access state variable;

        if(hitL1){  // If read hit in L1: directly return data and no need to access L2
          L1AcceState = RH;
          L2AcceState = NA;
        }
        else if(hitL2){  //read miss in L1 and read hit in L2: copy address in L2 to L1. If data in L1 needed to be evicted and is dirty, trigger a write access in L2
          L1AcceState = RM;
          L2AcceState = RH;  
          /*
          if(L1 is empty) update L1
          else{
            if(evicted data is clean) update L1
            else{  // if dirty, write access to L2
              if(write hit) do nothing about L2 tag, update L1
              else{  // write miss
                update L1, forward data to main memory without changing the L2 cache state
              }
            }
          }       
          */
          cacheL1.getCache()[counterL1[indexL1]][indexL1] = (unsigned long)tagL1; //put tag of L2 into L1
          cacheL1.getValidBit()[counterL1[indexL1]][indexL1] = true; //update valid bit
          counterL1[indexL1] = (counterL1[indexL1] + 1) % cacheconfig.L1setsize;  //update counter

        }
        else{ //read miss in both L1 and L2: update address in L1 and L2. if L2 needs eviction, L1 must be evicted too
          L1AcceState = RM;
          L2AcceState = RM;
          //update address in L2
          cacheL2.getCache()[counterL2[indexL2]][indexL2] = (unsigned long)tagL2;  //put tag in memory to L2  
          cacheL2.getValidBit()[counterL2[indexL2]][indexL2] = true; //update valid bit
          counterL2[indexL2] = (counterL2[indexL2] + 1) % cacheconfig.L2setsize;  //update counter
          //update address in L1
          cacheL1.getCache()[counterL1[indexL1]][indexL1] = (unsigned long)tagL1; //put tag in L2 to L1  
          cacheL1.getValidBit()[counterL1[indexL1]][indexL1] = true; //update valid bit
          counterL1[indexL1] = (counterL1[indexL1] + 1) % cacheconfig.L1setsize;  //update counter
        }
      }
      else 
      {    
        //Implement by you:
        // write access to the L1 Cache, 
        //and then L2 (if required), 
        //update the L1 and L2 access state variable;

        if(hitL1) {  // write hit in L1: keeps tag in L1, no change in L2
          L1AcceState = WH;
          L2AcceState = NA;
        } 
        else if(hitL2) {  //write miss in L1 and write hit in L2: on-allocate so write data to L2, no change in L1 and L2 tag
          L1AcceState = WM;
          L2AcceState = WH;      
        } 
        else{  //write miss in L1 and L2: on-allocate so write data to memory without changing tags in L1 and L2
            L1AcceState = WM;
            L2AcceState = WM;
        }
      }

      // Output hit/miss results for L1 and L2 to the output file
      tracesout<< L1AcceState << " " << L2AcceState << endl;
    }
    traces.close();
    tracesout.close(); 
  }
  else cout<< "Unable to open trace or traceout file ";
  return 0;
}


