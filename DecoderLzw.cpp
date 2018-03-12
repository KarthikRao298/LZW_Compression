/*
 * File Name : DecoderLzw.cpp
 * ver: 0.4
 * date : Mar 03 2018
 *
 * To compile :
 * make clean
 * make
 *
 * To compile :
 * g++ -std=c++11 DecoderLzw.cpp -o decoderLzw
 * 
 * Sample command line execution :
 * 
 * ./decoderLzw input1.txt 12
 *
 */

#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <string.h>
#include <cmath>
#include <unordered_map>
#include <cassert>

#include <fstream>
#include "CommonHeader.h"

#define MAX_ASCII_CODE 255
#define MAX_FILE_NAME 50

class Decoder
{
    public:
        Decoder();
        ~Decoder();


        CStatus DictInit ();
        std::string DictGetString (int code);
        bool IsInDict ( int code);

        CStatus DictInsert (std::string str);

        bool IsDictFull(int size);

    private:
        int noOfSymbols;
        std::unordered_map<int, std::string> dictWords;

};


/*==============================================================================
 *  main
 *=============================================================================*/

int main (int argc, char* argv[]) {

    if (argc < 3) {
        std::cerr<<"Usage: "<<argv[0]<<" <Encoded File Name>"<<" <bit Length>"<<std::endl;

        return -1;
    }

    DLOG (C_VERBOSE, "Enter\n");

    Decoder dec;
    char inputFile[MAX_FILE_NAME];
    char outputFile[MAX_FILE_NAME];
    int code;
    int bitLen;
    std::string str;
    char * lastDot;
    int tableSize;

    CStatus err;

    strcpy (inputFile, argv[1]);
    bitLen = atoi(argv[2]);
    tableSize = pow (2,bitLen);

    DLOG(C_VERBOSE,"Input file name:%s\n", inputFile);
    DLOG(C_VERBOSE,"Entered bit length:%d\n", bitLen);
    DLOG(C_VERBOSE,"Max table size = %d\n", tableSize);

    strcpy (outputFile,inputFile);

    lastDot = strrchr (outputFile, '.');
    if (lastDot != NULL){
        *lastDot = '\0';

        strcat (outputFile,"_decoded.txt");
    }
    DLOG(C_VERBOSE,"Output file name:%s\n", outputFile);

    std::ifstream inStream(inputFile, std::ios::binary);
    if (! inStream.is_open()) { 
        DLOG(C_ERROR,"Failed to open file : %s\n", inputFile);
        return -1;
    }
    std::ofstream onStream (outputFile, std::ios::out);
    if (! onStream.is_open()) { 
        DLOG(C_ERROR,"Failed to open file : %s\n", outputFile);
        return -1;
    }
    err = dec.DictInit ();
    if (err) {
        DLOG(C_ERROR,"DictInit failed\n");
    }

    inStream.read(reinterpret_cast<char*>(&code), sizeof(short int));

    if ((char)code == EOF){

        DLOG(C_ERROR,"Empty input file\n");

    }
    std::string str_ing;
    std::string new_string;
    str_ing = dec.DictGetString(code);
    onStream << str_ing;



    while (1) {

        inStream.read(reinterpret_cast<char*>(&code), sizeof(short int));


        if ( inStream.eof() ){
            DLOG (C_VERBOSE,"End of file encountered\n");

            break;
        }

        if (!dec.IsInDict(code)){

            DLOG (C_VERBOSE,"Code is not in the dict\n");
            new_string = str_ing + str_ing[0];

        }else{

            DLOG (C_VERBOSE,"Code found in the dict\n");
            new_string = dec.DictGetString(code);

        }
        DLOG (C_VERBOSE,"printing new_string sizeof(new_string)=%lu len=%lu to the outputFile--",sizeof(new_string),new_string.length());
        onStream << new_string;

        if (!dec.IsDictFull (tableSize)) {
            DLOG (C_VERBOSE,"Insering string the dict\n");
            dec.DictInsert (str_ing + new_string[0]);
            str_ing = new_string;
        }

    }

    inStream.close();
    onStream.close();
    DLOG (C_INFO, "Decoder Executed\n");
}



/*==============================================================================
 *  DictInit
 *=============================================================================*/

CStatus Decoder::DictInit () {
    /*
     * initialize the dictionary
     */
    int ch;
    std::string str;

    for (ch = 0; ch < MAX_ASCII_CODE + 1 ; ch++) {

        str = (char)ch;      
        dictWords[ch] = str;
    }

    noOfSymbols = MAX_ASCII_CODE;


    DLOG (C_VERBOSE, "dictWords.size = %lu\n",dictWords.size());
    DLOG (C_VERBOSE, "dictWords.max_size = %lu\n",dictWords.max_size());
    DLOG (C_VERBOSE, "dictWords.max_bucket_count = %lu\n",dictWords.max_bucket_count());
    DLOG (C_VERBOSE, "dictWords.bucket_count = %lu\n",dictWords.bucket_count());
    DLOG (C_VERBOSE, "dictWords.load_factor = %f\n",dictWords.load_factor());

    return C_SUCCESS;
}


/*==============================================================================
 *  DictGetString
 *=============================================================================*/
std::string Decoder::DictGetString (int code) {

    /*
     * get the string of the input code
     */

    std::string str;

    if (dictWords.find(code) != dictWords.end()) {
        DLOG (C_VERBOSE, "dictionary contains %d\n",code);

        std::unordered_map<int, std::string>::iterator i = dictWords.find(code);
        assert(i != dictWords.end());
        str = i->second;

    } else {
        DLOG (C_VERBOSE, "code (%d) not in dictionary !\n",code);
    }


    DLOG (C_VERBOSE, "Exit. Code = %d\n",code);
    return str;
}

/*==============================================================================
 *  IsInDict
 *=============================================================================*/
bool Decoder::IsInDict ( int code) {
    /*
     * check if the input code is available in the dictionary & return status accordingly
     */

    bool isFound = true;

    if (dictWords.find(code) != dictWords.end()) {
        DLOG (C_VERBOSE, "dictionary contains code(%d)!\n",code);

        std::unordered_map< int, std::string>::iterator i = dictWords.find(code);
        assert(i != dictWords.end());

    } else {
        DLOG (C_VERBOSE, " code (%d) not in dictionary !\n",code);
        isFound = false;
    }

    DLOG (C_VERBOSE, "Exit\n");
    return isFound;
}


/*==============================================================================
 *  DictInsert
 *=============================================================================*/

CStatus Decoder::DictInsert (std::string str) {
    /*
     * Insert the string str to the dictionary & return operation status
     */
    DLOG (C_VERBOSE, "inserting ");

    CStatus err = C_SUCCESS;

    noOfSymbols ++;
    dictWords[noOfSymbols] = str ;


    DLOG (C_VERBOSE, "Exit. noOfSymbols = %d\n",noOfSymbols);
    return err;
}

/*==============================================================================
 *  Decoder
 *=============================================================================*/
Decoder::Decoder() {

    noOfSymbols = 0;
}


/*==============================================================================
 *  ~Decoder
 *=============================================================================*/
Decoder::~Decoder() {

    /* do nothing */
}

/*==============================================================================
 *  Decoder
 *=============================================================================*/
bool Decoder::IsDictFull(int tableSize) {

    bool isFull = true;

    if (dictWords.size() < (unsigned long) tableSize) {
        isFull = false; 
    }
    return isFull;
}

