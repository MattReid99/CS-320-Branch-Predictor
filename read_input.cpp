#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include <vector>
#include <math.h>

#define T "T"
#define NT "NT"

using namespace std;

vector<unsigned long long> addresses;
vector<string> behaviors;

int alwaysTaken = 0, neverTaken = 0, num_bimodalOne = 0, num_bimodalTwo = 0;
int tableSizes[] = {16, 32, 128, 256, 512, 1024, 2048};


vector<long> bimodalOne() {

  vector<int> buffer;
  int tableSizes[] = {16, 32, 128, 256, 512, 1024, 2048};

  vector<long> retVal;

  for (int i=0; i<7; i++) {

    //SET SIZE OF TABLE
    buffer.resize(tableSizes[i]);

    int correct = 0, total = addresses.size();

    for (unsigned int j=0; j<total; j++) {

      int index = addresses[j] % tableSizes[i];
      if (buffer[index] == 1) {
        if (behaviors[j] == T) correct++;
        else buffer[index] = 0;
      }
      else {
        if (behaviors[j] == NT) correct++;
        else buffer[index] = 1;
      }
      buffer.clear();
    }
    retVal.push_back(correct);
    }
    return retVal;
  }



  vector<long> bimodalTwo() {
    // Temporary variables
    unsigned long long addr;
    string behavior, line;

    vector<int> buffer;
    int tableSizes[] = {16, 32, 128, 256, 512, 1024, 2048};

    vector<long> retVal;

    for (int i=0; i<7; i++) {

      //SET SIZE OF TABLE
      buffer.resize(tableSizes[i]);
      fill(buffer.begin(), buffer.end(), 1);

      int correct = 0, total = addresses.size();

      for (unsigned int j=0; j<total; j++) {

        int index = addresses[j] % tableSizes[i];
        int & prediction = buffer[index];
        bool isCorrect = ((behaviors[j] == T) == (prediction == 2 || prediction == 3));
        // 0 =
        if (isCorrect) {
            correct++;
            if (behaviors[j] == T) prediction = 3; //set strongly taken
            else if (behaviors[j] == NT) prediction = 0; //set strongly not taken
        }
        else {
          //cerr << "branch mispredicted" << endl;
          if (prediction == 1 || prediction == 3) prediction = 2; //set weakly taken
          else if (prediction == 2 || prediction == 0) prediction = 1; //set weakly not taken
        }
        buffer.clear();
      }
      retVal.push_back(correct);
      }
      return retVal;
    }

  vector<long> gshare() {
    // 0 = strongly NT, 1 = weakly NT, 2 = weakly T, 3 = strongly T
    int masks[] = {3,7,15,31,63,127,255,511,1023};
    vector<int> table(2048, 1);
    vector<long> retVal;

    int correct = 0, total = addresses.size(), maxBits = 3;
    string historyString;

    while (maxBits < 11) {
      correct = 0;
      fill(table.begin(), table.end(), 1);
      historyString = "";


      for (int m=0; m<maxBits; m++) historyString.append("0");

       for (unsigned int i=0; i<addresses.size(); i++) {

        unsigned long globalHistory = stoi(historyString, nullptr, 2);
        int index = (addresses[i] ^ globalHistory) % 2048;
        int & prediction = table[index];

        bool isCorrect = ((behaviors[i] == T) == (prediction == 2 || prediction == 3));
        if (isCorrect) {
          correct++;
          if (behaviors[i] == T) prediction = 3;
          else if (behaviors[i] == NT) prediction = 0;
        }
        else {
          if (prediction == 1 || prediction == 3) prediction = 2;
          else if (prediction == 0 || prediction == 2) prediction = 1;
        }

        //rotate(historyBits.begin(), historyBits.begin()+1, historyBits.end());
        if (behaviors[i] == T) historyString = historyString.substr(1, historyString.size()) + "1";
        else historyString = historyString.substr(1, historyString.size()) + "0";
      }
      retVal.push_back(correct);
      maxBits++;
    }
    return retVal;
  }

  unsigned long tournamentPredictor() {
    //0 – prefer Gshare, 1 – weakly prefer Gshare, 2 – weakly prefer Bimodal, 3 – prefer bimodal
    unsigned long correct = 0;
    vector<int> gshareTable(2048, 1);

    vector<int> bimodalTable(2048, 1);

    vector<int> selectorTable(2048);
    fill(selectorTable.begin(), selectorTable.end(), 2);
    string historyString = "00000000000";


    for (int i=0; i<addresses.size(); i++) {
      //cerr << "globalHistory = " << historyString;
      unsigned long long globalHistory = stoi(historyString, nullptr, 2);

      int indexGshare = ((addresses[i] & 2047) ^ globalHistory) % 2048;
      // right most 11 bits
      int index = addresses[i] % 2048;
      int & selection = selectorTable[index];

      int & bimodalGuess = bimodalTable[index];
      int & gshareGuess = gshareTable[indexGshare];

      bool gshareCorrect = ((behaviors[i] == T) == (gshareGuess >= 2));
      bool bimodalCorrect = ((behaviors[i] == T) == (bimodalGuess >= 2));

      if ((selection == 2 || selection == 3) && bimodalCorrect) correct++;
      if ((selection == 0 || selection == 1) && gshareCorrect) correct++;

      if (gshareCorrect) {
        if (behaviors[i] == T) gshareGuess = 3;
        else gshareGuess = 0;
      }
      else {
        if (gshareGuess == 0 || gshareGuess == 2) gshareGuess = 1;
        else gshareGuess = 2;
      }

      if (bimodalCorrect) {
        if (behaviors[i] == T) bimodalGuess = 3;
        else bimodalGuess = 0;
      }
      else {
        if (bimodalGuess == 0 || bimodalGuess == 2) bimodalGuess = 1;
        else bimodalGuess = 2;
      }

      //if predictions are not same, decrement/increment accordingly
      if ((gshareCorrect && !bimodalCorrect) || (!gshareCorrect && bimodalCorrect)) {
        if (gshareCorrect) selection -= 1;
        if (selection < 0) selection = 0;
        if (bimodalCorrect) selection += 1;
        if (selection > 3) selection = 3;
      }

      if (behaviors[i] == T) historyString = historyString.substr(1, historyString.size()) + "1";
      else historyString = historyString.substr(1, historyString.size()) + "0";
    }
    return correct;
  }


  int main(int argc, char *argv[]) {
    if (argc != 3) {
      cerr << "correct arguments:\t./predictors <input file>.txt <output file>.txt\n";
      return 1;
    }
    remove(argv[2]);

    unsigned long long addr;
    string behavior, line;
    int total = 0;

    ifstream infile(argv[1]);
    ofstream outputFile(argv[2], ios::app);

    while(getline(infile, line)) {
      total++;
      stringstream s(line);
      s >> std::hex >> addr >> behavior;
      addresses.push_back(addr);
      behaviors.push_back(behavior);

      if (behavior == T) alwaysTaken++;
      else neverTaken++;
    }

    outputFile << alwaysTaken << "," << total << ";" << endl;
    outputFile << neverTaken << "," << total << ";" << endl;
    vector<long> bmoneResult = bimodalOne();
    vector<long> bmtwoResult = bimodalTwo();
    vector<long> gshareResult = gshare();
    for (long i : bmoneResult) outputFile << i << "," << total << "; ";
    outputFile << endl;

    for (long i : bmtwoResult) outputFile << i << "," << total << "; ";
    outputFile << endl;

    for (long i : gshareResult) outputFile << i << "," << total << "; ";
    outputFile << endl;

    outputFile << tournamentPredictor() << "," << total << ";" << endl;
    outputFile.close();
    //bimodalOne("traces/short_trace1.txt");
    //bimodalTwo("traces/short_trace1.txt");
    //gshare();
    //tournamentPredictor();
    return 0;
  }
