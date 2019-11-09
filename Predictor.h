#ifndef PREDICTOR_H
#define PREDICTOR_H
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

struct entry {
	bool taken;
	int address;
	int target;
};

class Predictor {
	public:
		Predictor(std::string, std::string);
		void alwaysTaken();
		void alwaysNotTaken();
		void bimodalSingleBit(int tsize);
		void bimodalTwoBits(int tsize);
		void gShare(int ghr_size);
		void tournament();
		void branchTargetBuffer();
		void getEntries();
		void output(std::string);
	private:
		std::vector<entry> entries;
		std::ofstream ofile;
		int num_taken;
		int num_nt;
		int num_branches;
};

#endif
