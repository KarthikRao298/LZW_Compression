# Objective

The Lempel–Ziv–Welch  (LZW)  algorithm is  a  lossless  data  compression  algorithm.  LZW  is  an  adaptive  compression  algorithm  that  does  not  assume  prior  knowledge  of  the  input  data  distribution.  This algorithm works well when the input data is sufficiently large and there is redundancy in the data.

The  encoding  table,  or  dictionary, computed  during  the  encoding  process  does not need to be explicitly transmitted. It can be regenerated from the coded/compressed data.

The objective is to implement encoder & decoder for the LZW algorithm.
