#ifndef _M3_MACRO_H
#define _M3_MACRO_H

#define DEBUG_LEVEL 0
#define BEGIN_DEBUG if (DEBUG_LEVEL){
#define BEGIN_DEBUG_0 if (DEBUG_LEVEL==0){
#define BEGIN_DEBUG_1 if (DEBUG_LEVEL<=1){
#define BEGIN_DEBUG_2 if (DEBUG_LEVEL<=2){
#define BEGIN_DEBUG_3 if (DEBUG_LEVEL<=3){
#define END_DEBUG }
#define TIME_DEBUG_OUT debug_out << "TIME " <<  MPI_Wtime()-m3_start_time << "s ~~~~~: "


//*****
// all data in MPI is size=4,so the pointer in 64PC must be explit to 2data

//define the pointer in mpi.
//32PC
#define MPI_POINTER MPI_INT
#define MPI_POINTER_LENGTH 1

//64PC
//#define MPI_POINTER MPI_DOUBLE
//#define MPI_POINTER_LENGTH 2


#endif
