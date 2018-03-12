/*
 * File Name : EncoderLzw.cpp
 * ver: 0.5
 * date : Mar 03 2018
 *
 * To compile :
 * make clean
 * make
 *
 * To compile :
 * g++ -std=c++11 LzwEncoder.cpp -o lwzEncoder
 * 
 * Sample command line execution :
 * 
 * ./lxwEncoder input1.txt 12
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


class Encoder
{
    public:
        Encoder();
        ~Encoder();

        CStatus DictInit ();

        CStatus DictInsert (std::string str);

        bool IsInDict (std::string str);

        int DictGetCode (std::string str);

        bool IsDictFull(int size);

    private:
        int noOfSymbols;
        std::unordered_map<std::string, int> dictWords;

};

/*==============================================================================
 *  main
 *=============================================================================*/

int main (int argc, char* argv[]) {


    if (argc < 3) {
        std::cerr<<"Usage: "<<argv[0]<<" <input File Name>"<<" <bit Length>"<<std::endl;

        return -1;
    }

    DLOG (C_VERBOSE, "Enter\n");


    Encoder enc;
    char inputFile[15];
    char outputFile[15];
    FILE *iFPtr;
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
    if (lastDot != NULL)
        *lastDot = '\0';

    strcat (outputFile,".lzw");


    iFPtr = fopen(inputFile, "r");
    if (iFPtr == NULL)
    {
        DLOG(C_ERROR,"Failed to open file : %s\n", inputFile);
        return -1;
    }

    std::ofstream oFPtr (outputFile, std::ios::binary);


    err = enc.DictInit ();
    if (err) {
        DLOG(C_ERROR,"DictInit failed\n");
    }

    std::string str_ing;
    char symbol;
    while (1) {

        symbol = fgetc (iFPtr);

        if (symbol == EOF){
            DLOG (C_VERBOSE,"End of file encountered\n");
            code = enc.DictGetCode (str_ing);
            DLOG (C_VERBOSE,"printing code (%d)to the outputFile\n",code);

            oFPtr.write(reinterpret_cast<const char*>(&code), sizeof(short int));
            break;
        }


        if (enc.IsInDict(str_ing+symbol)) {
            str_ing += symbol;
        } else {
            code = enc.DictGetCode (str_ing);
            DLOG (C_VERBOSE,"printing code (%d)to the outputFile\n",code);

            oFPtr.write(reinterpret_cast<const char*>(&code), sizeof(short int));

            if (!enc.IsDictFull (tableSize)) {

                enc.DictInsert (str_ing + symbol);

            }
            str_ing = symbol;

        }

    }
    oFPtr.close();

    fclose(iFPtr);
    DLOG (C_INFO, "Encoder executed\n");
}


/*==============================================================================
 *  DictGetCode
 *=============================================================================*/
int Encoder::DictGetCode (std::string str) {

    /*
     * get the code of the input string
     */

    DLOG (C_VERBOSE, "Enter\n");
    int code = 0;

    if (dictWords.find(str) != dictWords.end()) {
        DLOG (C_VERBOSE, "dictionary contains string\n");

        std::unordered_map<std::string, int>::iterator i = dictWords.find(str);
        assert(i != dictWords.end());
        code = i->second;

    } else {
        DLOG (C_VERBOSE, " string not in dictionary !\n");
    }


    DLOG (C_VERBOSE, "Exit. Code = %d\n",code);
    return code;
}


/*==============================================================================
 *  IsInDict
 *=============================================================================*/
bool Encoder::IsInDict ( std::string str) {
    /*
     * check if the input str is available in the dictionary & return status accordingly
     */

    DLOG (C_VERBOSE, "Enter\n");
    bool isFound = true;

    if (dictWords.find(str) != dictWords.end()) {
        DLOG (C_VERBOSE, "dictionary contains string!\n");

    } else {
        DLOG (C_VERBOSE, "string not in dictionary !\n");
        isFound = false;
    }

    DLOG (C_VERBOSE, "Exit\n");
    return isFound;
}


/*==============================================================================
 *  DictInsert
 *=============================================================================*/

CStatus Encoder::DictInsert (std::string str) {
    /*
     * Insert the string str to the dictionary & return operation status
     */
    DLOG (C_VERBOSE, "inserting string\n");

    CStatus err = C_SUCCESS;

    dictWords[str] = ++ noOfSymbols ;


    DLOG (C_VERBOSE, "Exit. noOfSymbols = %d\n",noOfSymbols);
    return err;
}

/*==============================================================================
 *  DictInit
 *=============================================================================*/

CStatus Encoder::DictInit () {
    /*
     * initialize the dictionary
     */
    int ch;
    std::string str;

    for (ch=0; ch<MAX_ASCII_CODE +1 ; ch++) {

        str = (char)ch;      
        dictWords[str] = ch;
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
 *  Encoder
 *=============================================================================*/
Encoder::Encoder() {

    noOfSymbols = 0;
}


/*==============================================================================
 *  ~Encoder
 *=============================================================================*/
Encoder::~Encoder() {

    /* do nothing */
}

/*==============================================================================
 *  Encoder
 *=============================================================================*/
bool Encoder::IsDictFull(int tableSize) {

    bool isFull = true;

    if (dictWords.size() < (unsigned long) tableSize) {
        isFull = false; 
    }
    return isFull;
}


