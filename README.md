# qkdDecomposer

/***************************************************************************
 * 3. Decomposition of timestamp and basis information: in quantum key distribution, 
 * we locally measure a series of photon arrival times with one out of four possible outcomes. 
 * Each photon arrival event receives a timestamp (integer of the current time in nanoseconds) 
 * and a detector click (1-4). The two are combined and stored in a large binary data file 
 * (much like the one attached as “test3_Alice”) as 64 bit integers (or rather 2x 32 bit integers). 
 * Could you write a short C/C++ program to read the file, split the timestamp and detector click information, 
 * and store them in separate files? Measure how fast the read/processing/write processes are. 
 * What is the average separation of time stamps in the attached file?
 *
 * The data is generated from two arrays (stamp = time stamps, basis= detector clicks) 
 * in python with the function in saveBinary.py. By understanding this function, 
 * you should be able to decipher the data format.
 *
 * VolcanBaruChiriqui's solution using gcc 5.4.0 on Ubuntu 16.04:
 * 
 * - compile using command: gcc qkdDecomposer.c -lm -o qkdDecomposer 
 *
 * - in qkdDecomposer.h, add/remove preprocessor definition HUMAN_READABLE_OUTPUT
 *   to save decomposed info in text/binary format.
 *
 * - add/remove following preprocessor definitions to measure the respective timings
 *
 *	#define MEASURE_READ_TIME
 *	#define MEASURE_PROCESS_TIME
 *	#define MEASURE_WRITE_TIME
 ***************************************************************************/
