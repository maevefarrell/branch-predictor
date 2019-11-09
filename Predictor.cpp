#include "Predictor.h"
#include <stdlib.h>
#include <iostream>
#include<fstream>
#include<sstream>
#include <vector>
#include <string>
#include <math.h>

using namespace std;

Predictor::Predictor(string ifilename, string ofilename) {
	this->num_branches = 0;
	
	unsigned long long addr;
  	std::string behavior, line;
  	unsigned long long target;
  	
  	ifstream infile(ifilename);
  	//ofstream ofile;
  	this->ofile.open(ofilename);
  	
  	while(getline(infile, line)) {
  		this->num_branches++;
  		stringstream s(line);
  		s >> std::hex >> addr >> behavior >> std::hex >> target;
  		
  		entry e;
  		e.address = addr;
  		e.target = target;
  		
  		if(behavior == "T") {
  			e.taken = true;
  			this->num_taken++;
  		}
  		else {
  			e.taken = false;
  			this->num_nt++;
  		}
  		
  		this->entries.push_back(e);
  	}
}

void Predictor::alwaysTaken() {

	int correct_preds = 0;
	for(entry e : entries) {
		if(e.taken) {
			correct_preds++;
		}
	}
	this->ofile << correct_preds << "," << this->num_branches << ";" << endl;
}

void Predictor::alwaysNotTaken() {
	int correct_preds = 0;
	for(entry e : entries) {
		if(!e.taken) {
			correct_preds++;
		}
	}
	this->ofile << correct_preds << "," << this->num_branches << ";" << endl;
}

void Predictor::bimodalSingleBit(int table_size) {
	
	//initializing table
	vector<bool> table(table_size, true);
	
	int correct_preds = 0;
	for(entry e : entries) {
	
		int key = e.address % table_size;
		
		if(e.taken == table[key]) {
			correct_preds++;
		}
		
		else {
			if(table[key]) {
				table[key] = false;
			}
			
			else {
				table[key] = true;
			}
		}
	}
	
	this->ofile << correct_preds << "," << this->num_branches << "; ";
}

void Predictor::bimodalTwoBits(int table_size) {

	vector<int> table(table_size, 3);
	
	int correct_preds = 0;
	
	for(entry e : entries) {
	
		int key = e.address % table_size;
		
		if(e.taken && table[key] > 1) {
			correct_preds++;
			if(table[key] < 3) table[key]++;
		}
		
		else if (e.taken && table[key] < 2) {
			if(table[key] < 3) table[key]++;
		}
		
		else if(!e.taken && table[key] > 1) {
			if(table[key] > 0) table[key]--;
		}
		
		else if(!e.taken && table[key] < 2){
			correct_preds++;
			if(table[key] > 0) table[key]--;
		}
	}
	
	this->ofile << correct_preds << "," << this->num_branches << "; ";
	
}

void Predictor::gShare(int ghr_size) {
	
	int correct_preds = 0;
	
	int ghr = 0<<ghr_size;
	vector<int> table(2048, 3);
	
	int size = pow(2, ghr_size) - 1;
	
	for(entry e : entries) {
	
		int key = (e.address ^ (ghr&size)) % 2048;
		
		if(e.taken && table[key] > 1) {
			correct_preds++;
			if(table[key] < 3) table[key]++;
			ghr = ghr<<1;
			ghr += 1;
		}
		
		else if (e.taken && table[key] < 2) {
			if(table[key] < 3) table[key]++;
			ghr = ghr<<1;
			ghr += 1;
		}
		
		else if(!e.taken && table[key] > 1) {
			if(table[key] > 0) table[key]--;
			ghr = ghr<<1;
		}
		
		else if(!e.taken && table[key] < 2){
			correct_preds++;
			if(table[key] > 0) table[key]--;
			ghr = ghr<<1;
		}
	}
	
	this->ofile << correct_preds << "," << this->num_branches << "; ";

}

void Predictor::tournament() {
	vector<int> selector_table(2048, 0);
	
	vector<int> gshare_table(2048, 3);

	vector<int> bimodal_table(2048, 3);
	int correct_preds = 0;
	
	int ghr = 0;
	
	int size = pow(2, 11) - 1;
	
	//tournament logic
	
	for(entry e : entries) {
		int key = e.address % 2048;
		int gkey = (e.address ^ (ghr&size)) % 2048;
		
		if(selector_table[key] == 0 || selector_table[key] == 1) { //predict gshare
			//prefer gshare
			if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3 ) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { //both correct
				correct_preds++;
			}
			
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && e.taken) { //gshare correct, bimodal incorrect
				correct_preds++;
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3) selector_table[key]--;
			}
			
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { //gshare incorrect, bimodal correct
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
			
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { //both correct
				correct_preds++;
			}
			
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && !e.taken) { //gshare correct, bimodal incorrect
				correct_preds++;
				
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3) selector_table[key]--;
			}
			
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { //bimodal correct, gshare incorrect
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
		}
		
		else if(selector_table[key] == 2 || selector_table[key] == 3) { //predict bimodal
			if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { //both correct
				correct_preds++;
			}
			
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { //bimodal correct, ghsare incorrect
				correct_preds++;
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
			
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && e.taken) { //bimodal incorrect, gshare correct
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3)  selector_table[key]--;
				
			}
			
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { //both correct
				correct_preds++;
			}
			
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && !e.taken) { //gshare correct, bimodal incorrect
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3) selector_table[key]--;
			}
			
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { //gshare incorrect, bimodal correct
				correct_preds++;
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
		}

		else {
			cout << "preferring neither gshare nor bimodal" << endl;
		}

		if(e.taken) {
			if(gshare_table[gkey] < 3) gshare_table[gkey]++;
			ghr = ghr<<1;
			ghr += 1;
			if(bimodal_table[key] < 3) bimodal_table[key]++;
		}

		else {
			if(gshare_table[gkey] > 0) gshare_table[gkey]--;
			ghr = ghr<<1;
			if(bimodal_table[key] > 0) bimodal_table[key]--;
		}
	}
	
	this->ofile << correct_preds << "," << this->num_branches << "; " << endl;
}

void Predictor::branchTargetBuffer() {
	int accesses = 0;
	int correct_preds = 0;
	
	vector<bool> predictions(512, true);
	vector<pair<unsigned long long, unsigned long long>> btb(128);
	
	for(entry e : entries) {
		int key = e.address % 512;
		int btb_index = e.address % 128;
		
		if(predictions[key] == true) {
			accesses++;
			if(btb[btb_index].first == e.address) {
				if(btb[btb_index].second == e.target) {
					correct_preds++;
				}
				
				else {
					btb[btb_index].second = e.target;
				}
			}
		
			else {
				btb[btb_index].first = e.address;
				btb[btb_index].second = e.target;
			}
			
		}
		
		predictions[key] = e.taken;
		
	}

	this->ofile << accesses << "," << correct_preds << ";" << endl;
	this->ofile.close();
	
}

void Predictor::getEntries() {
	for(entry e : entries) {
		std::cout << "taken?: " << e.taken << " " << "address: " << e.address << endl;
	}
}

void Predictor::output(string s) {
	this->ofile << s;
}
