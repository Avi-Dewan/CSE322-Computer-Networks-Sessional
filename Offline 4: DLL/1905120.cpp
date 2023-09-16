#include<iostream>
#include<string>
#include <vector>
#include <bitset>
#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include <time.h>
#include <unordered_set>
using namespace std;

string addPadding(string s, int m) {
    while (true )
    {
        if(s.size() % m == 0) break;
        s += "~";
    }
    return s;
}


vector<vector <int>> createDataBlock(string s, int m) {
    vector<vector<int>> datablock;
    
    // Calculate the number of rows needed (l/m rows)
    int l = s.length();
    int numRows = l / m;

    for (int i = 0; i < numRows; i++) {

        vector<int> row;

        for (int j = 0; j < m; j++) {

            char c = s[i * m + j];

            int asciiValue = static_cast<int>(c);

            bitset<8> binary(asciiValue);

            // cout << binary << endl;
            // for(int k = 0; k < 8; k++) cout << binary[k];
            // cout << endl;

            for (int k = 7; k >=0; k--) { // msb on left side ( index 7), lsb on right side
                row.push_back(binary[k]); 
            }
        }
        datablock.push_back(row);
    }

    return datablock;
}


int calculateR(int m){

    int r = 0;

    while(m * 8 + r + 1 > pow(2 , r)){
        r++;
    }
    
    return r;
}

vector<vector<int>> addCheckBit(vector<vector<int>> datablock, int r) {
    
    for(int i = 0; i < datablock.size(); i++) {
        //adding then
        int r_index = 1;
        for(int j = 0; j < r; j++) {
            datablock[i].insert(datablock[i].begin() + r_index -1 , 0);
            r_index *= 2;
        }

        r_index = 1;
        for(int j = 0 ; j < r ; j++){
            int count = 0;
            for(int k = r_index + 1 ; k <= datablock[i].size() ; k++){
                if((r_index & k) != 0){
                    count += datablock[i][k-1];
                }
            }

            //instead of x-oring everytime i just count total 1.
            // Odd one -> parity = 1
            // Else -> parity = 0

            if(count % 2 != 0) 
                datablock[i][r_index-1] = 1;

            r_index*=2;
        }
    }
    return datablock;
}

vector<int> serialzieColumnwise(vector<vector<int>> datablock) {
    vector<int> serializedBlock;

    for(int j = 0; j < datablock[0].size(); j++) {
        for(int i = 0; i < datablock.size(); i++) {
            serializedBlock.push_back(datablock[i][j]);
        }
    }

    return serializedBlock;
}

vector<int> calculateChecksum(vector<int> serialziedData, string gen_pol) {

    vector<int> checksum;

    int checkSumSize = gen_pol.size() - 1;
    int checkSumStart = serialziedData.size();

    for(int i = 0; i < checkSumSize; i++) serialziedData.push_back(0);

    for(int i = 0; i <= (serialziedData.size() - gen_pol.size()); i++) {
        if(serialziedData[i]) {
            for(int j = 0; j < gen_pol.size(); j++) {
                serialziedData[i+j] = (serialziedData[i+j] ^ (gen_pol[j] - '0'));
            }
        }
    }

    for(int i = 0; i < checkSumSize; i++) {
        checksum.push_back(serialziedData[checkSumStart + i]);
    }


    return checksum;
}


vector<int> appendCheckSum(vector<int> serialziedData, vector<int> checksum) {
    for(int i = 0; i < checksum.size(); i++) {
        serialziedData.push_back(checksum[i]);
    }

    return serialziedData;
}

bool verifyChecksum(vector<int> serialziedData, string gen_pol) {
    bool isCorrect = true;

    for(int i = 0; i <= (serialziedData.size() - gen_pol.size()); i++) {
        if(serialziedData[i]) {
            for(int j = 0; j < gen_pol.size(); j++) {
                serialziedData[i+j] = (serialziedData[i+j] ^ (gen_pol[j] - '0'));
            }
        }
    }

    for(int i = 0; i < serialziedData.size(); i++ ) {
        if(serialziedData[ i]) {
            isCorrect = false;
            break;
        }
    }
    
    return isCorrect;    
}

vector<vector<int>> reconstructDatablock(vector<int> serialziedData, string gen_pol, int columnSize) {

    int checksumSize = gen_pol.size() - 1;
    int realSerializeSize = serialziedData.size() - checksumSize;

    int numRows = realSerializeSize / columnSize;

    vector<vector<int>> datablock;

    for(int i = 0; i < numRows; i++) {
        vector<int> row;
        for(int j = 0; j < columnSize; j++) {
            row.push_back(serialziedData[j*numRows + i]);
            // cout << row[j];
        }

        // cout << endl;

        datablock.push_back(row);
    }

    // cout << endl;

    return datablock;
}

vector<vector<int>>  removeCheckBit(vector<vector<int>> retrievedDatablock) {
    
    vector<vector<int>> datablock;

    for(int i = 0 ; i < (retrievedDatablock.size()) ; i++){
        int r_index = 1;
        vector<int> row;
        for(int j = 0 ; j < retrievedDatablock[i].size() ; j++){
            if(j == r_index-1){
                r_index *= 2;
            }
            else{
                row.push_back(retrievedDatablock[i][j]);
            }
        }
        datablock.push_back(row);
    }
    return datablock;
}

vector<vector<int>> correctError(vector<vector<int>> datablock, int r) {
    
    // re-calculate check bit -> They should be zero if no error

    for(int i = 0; i < datablock.size(); i++) {
        int r_index = 1;
        for(int j = 0 ; j < r ; j++){
            int count = datablock[i][r_index-1];
            for(int k = r_index + 1 ; k <= datablock[i].size() ; k++){
                if((r_index & k) != 0){
                    count += datablock[i][k-1];
                }
            }

            //instead of x-oring everytime i just count total 1.
            // Odd one -> parity = 1
            // Else -> parity = 0

            // if error -> 1
            // no error -> 0

            if(count % 2 != 0) 
                datablock[i][r_index-1] = 1;
            else 
                datablock[i][r_index-1] = 0;

            r_index*=2;
        }
    }


    // find Flipped index

    for(int i = 0; i < datablock.size(); i++) {
        int flippedIdx = 0;
        bool isflipped = false;

        int r_index = 1;

        for(int j = 0; j < r; j++) {

            if(datablock[i][r_index-1]) {
                flippedIdx += r_index;
                isflipped = true;
            }
            r_index*=2;
        }

        // cout << flippedIdx << " " << datablock[i].size() << endl;

        if(isflipped) {
            datablock[i][flippedIdx-1] ^= 1; 
        }
    }





    return datablock;
}


void printMessageinAscii(vector<vector<int>> datablock) {
    string msg = "";
    for(int i = 0; i < datablock.size(); i++) {
        for(int j = 0; j < datablock[i].size(); j += 8) {

            bitset<8> binary;

            for(int k = 0; k < 8; k++) {
                binary[7-k] = datablock[i][j+k];
            }

            msg.push_back((char)binary.to_ulong());
        }
    }

    cout << msg << endl;
}


int main() {

    srand(1);

    string str = "Hamming Code"; // data string, which is the string to be transmitted.
    int m = 2; // m, which indicates the number of bytes in a row of the data block. 
    double p = 0.05; // p, which indicates the probability of each bit being toggled during the transmission.
    string generator_polynomial = "10101"; //generator polynomial, which is used for calculating and verifying CRC checksum.

    cout << "enter data string: ";
    getline(cin, str);

    cout << "enter number of data bytes in a row <m>: ";
    cin >> m;

    cout << "enter probability <p>: ";
    cin >> p;

    cout << "enter generator polynomial: ";
    cin >> generator_polynomial;

    str = addPadding(str, m);

    cout << "data string after padding: " << str << "\n" << endl;

    // Task 2: Create the data block

    vector<vector<int>> datablock = createDataBlock(str, m);

    cout << "data block <ascii code of m characters per row>: " << endl;

    for(int i = 0 ; i< datablock.size() ; i++){
        for(int j = 0 ; j < datablock[i].size() ; j++){
            cout << datablock[i][j];
        }
        cout<<endl;
    }

    // Task 3: Add check bits

    int r = calculateR(m);
    datablock = addCheckBit(datablock, r);
    
    cout << "\ndata block after adding check bits: \n";

    for(int i = 0 ; i < (datablock.size()) ; i++){
        int r_index = 1;
        for(int j = 0 ; j < datablock[i].size() ; j++){
            if(j == r_index-1){
                printf("\033[1;32m%d\033[0m", datablock[i][j]);
                r_index *= 2;
            }
            else{
                cout << datablock[i][j];
            }
        }
        cout << endl;
    }

    // Task 4: Serialize the above data block in column-major manner. 

    vector<int> serialziedData = serialzieColumnwise(datablock);
    
    cout << "\ndata bits after column-wise serialization: "<<endl;

    for(int i = 0 ; i < serialziedData.size() ; i++){
        cout << serialziedData[i];
    }

    cout << endl;

    // Task 5: Compute the CRC checksum and Append the checksum to the bit stream.

    vector<int> checkSum = calculateChecksum(serialziedData, generator_polynomial);
    serialziedData = appendCheckSum(serialziedData, checkSum);
    
    cout<<"\ndata bits after appending CRC checksum <sent frame>: "<<endl;

    for(int i = 0 ; i < serialziedData.size() ; i++){
        if(i < serialziedData.size() - checkSum.size())
            cout << serialziedData[i];
        else
            printf("\033[1;36m%d\033[0m", serialziedData[i]);

    }

    cout << endl;

    // Task 6: Simulate the physical transmission by toggling each bit of the stream with a probability of p. Print the received frame
    
    double randomNumber;
    unordered_set<int> flippedIndex;

    // cout << serialziedData.size() << endl;

    for(int i = 0; i < serialziedData.size(); i++) {
        randomNumber = (double)rand() / RAND_MAX;
        if(randomNumber <= p) {
            serialziedData[i] ^= 1;
            flippedIndex.insert(i);
        }
    }

    cout << "\nreceived frame: " << endl;

    // cout << serialziedData.size() << endl;


    for(int i = 0; i < serialziedData.size(); i++) {
        if(flippedIndex.find(i) != flippedIndex.end()) {
            printf("\033[1;31m%d\033[0m", serialziedData[i]);
        } else {    
            cout << serialziedData[i];
        }
    }
    
    cout << endl;


    // Task 7: Verify the correctness of the received frame using the generator polynomial
    cout << "\nresult of CRC checksum matching: ";
    bool isCorrect = verifyChecksum(serialziedData, generator_polynomial);

    if(isCorrect) {
        cout << "no error detected" << endl;
    } else {
        cout << "error detected" << endl;
    }

    cout << endl;

    // Task 8: Remove the CRC checksum bits from the data stream and de-serialize it into the data-block in a column-major fashion.

    vector<vector<int>> retrievedDatablock = reconstructDatablock(serialziedData, generator_polynomial, m*8+r);
    
    cout << "\ndata block after removing CRC checksum bits: "<<endl;

    int numRows = retrievedDatablock.size();

    for(int i = 0; i < retrievedDatablock.size(); i++) {
        for(int j = 0; j < retrievedDatablock[i].size(); j++) {
            if(flippedIndex.find(j*numRows + i) != flippedIndex.end()) {
                printf("\033[1;31m%d\033[0m", retrievedDatablock[i][j]);
            } else {
                cout << retrievedDatablock[i][j];
            }
        }
        cout << endl;
    }

    // Task 9: Correct error[if there is more than one error in a row, this error correction mechanism will fail to correct the error]

    cout << "\ndata block after removing checkbits: "<<endl;
    vector<vector<int>> datablockWithoutCheckbits = removeCheckBit(retrievedDatablock);
    
    for(int i = 0; i < datablockWithoutCheckbits.size(); i++) {
        for(int j = 0; j < datablockWithoutCheckbits[i].size(); j++) {
            cout << datablockWithoutCheckbits[i][j];
        }
        cout << endl;
    }

    // Task 10: From the bits of the data block, compute the ascii codes of the characters. Print the data string.
    
    cout << "\noutput frame: ";

    vector<vector<int>> correctedDatablock = correctError(retrievedDatablock, r);
    vector<vector<int>> correctedDatablockwithoutCheckbit = removeCheckBit(correctedDatablock);

    printMessageinAscii(correctedDatablockwithoutCheckbit);


    return 0;
}