#ifndef _M3_MACRO_H
#define _M3_MACRO_H

#define DEBUG_LEVEL 0
#define BEGIN_DEBUG {
#define BEGIN_DEBUG_0 if (DEBUG_LEVEL==0){
#define BEGIN_DEBUG_1 if (DEBUG_LEVEL<=1){
#define BEGIN_DEBUG_2 if (DEBUG_LEVEL<=2){
#define BEGIN_DEBUG_3 if (DEBUG_LEVEL<=3){
#define END_DEBUG }
#define TIME_DEBUG_OUT debug_out << "TIME " <<  MPI_Wtime()-m3_start_time << "s ~~~~~: "


#endif