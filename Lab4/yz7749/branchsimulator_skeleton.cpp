#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <bitset>
using namespace std;

//define predictor states
#define ST 11 // Strong Taken
#define WT 10 // Weak Taken
#define WN 01 // Weak Not Taken
#define SN 00 // Strong Not Taken


int main (int argc, char** argv) {
	ifstream config;
	config.open(argv[1]);

	int m;
	config >> m;

    //initialize counters with ST
	unsigned long index = (unsigned long)pow(2, m);
    vector<unsigned long> counter(index, ST);
 
	config.close();

	ofstream out;
	string out_file_name = string(argv[2]) + ".out";
	out.open(out_file_name.c_str());
	
	ifstream trace;
	trace.open(argv[2]);

    int incorrect = 0;
    int total = 0;
	float misRate;

	while (!trace.eof()) {
		unsigned long pc; bool taken;
		trace >> std::hex >> pc >> taken;
		bitset<32> myPC = pc;

		// counters indexed using m LSBs of each branch instruction 
		string pcLsb = myPC.to_string().substr(32 - m, m); //substract m lsb from pc
		unsigned long counterIndex = bitset<32>(pcLsb).to_ulong();  //get counter index

		bool prediction;
		prediction = true;
		
		if(taken){  // SN -> WN -> WT -> ST
			if(counter[counterIndex] == ST || counter[counterIndex] == WT){  //current prediction
				prediction = 1;  //current prediction taken in 1 bit
				counter[counterIndex] = ST; //update predition for next time		
			}
			else if(counter[counterIndex] == WN){
				prediction = 0;
				counter[counterIndex] = WT;
				incorrect++;
			}
			else if(counter[counterIndex] == SN){
				prediction = 0;
				counter[counterIndex] = WN;
				incorrect++;
			}
		}
		else{  // not taken: ST -> WT -> WN -> SN
			if(counter[counterIndex] == ST){  //current prediction
				prediction = 1;
				counter[counterIndex] = WT; //update predition for next time
				incorrect++;		
			}
			else if(counter[counterIndex] == WT){
				prediction = 1;
				counter[counterIndex] = WN;
				incorrect++;
			}
			else if(counter[counterIndex] == WN || counter[counterIndex] == SN){
				prediction = 0;
				counter[counterIndex] = SN;
			}
		}
		total++;
		out << prediction << endl;
	}
	misRate = (float)incorrect / (float)total;
	cout << "m = " << m << endl;
    cout << "incorrect: " << incorrect << "    total: " << total << "   misprediction rate:" << misRate << endl;
	 
	trace.close();	
	out.close();
	return 0;
}
