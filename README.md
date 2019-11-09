## C++ Branch Predictor
This program implements a number of different branch prediction schemes using C++. It was tested using trace files ranging from 2 million to 10 million branch instructions.

Branch prediction schemes used:
1. Always assumes taken
2. Always assumes not-taken
3. Bimodal predictor with single bit of memory
4. Bimodal predictor with two bits of memory
5. GShare
6. Tournament (combined GShare and bimodal-two-bit)
7. Branch Target Buffer

This program was written for CS320: Advanced Computer Architecture at Binghamton University.
