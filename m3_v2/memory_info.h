#ifndef _MEMORY_INFO_H
#define _MEMORY_INFO_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
using namespace std;

class Memory_Info{
public:

  Memory_Info(){
    proc_meminfo="/proc/meminfo";
    meminfo="meminfo";
  }

  void set_meminfo_file(string f_name){
    proc_meminfo=f_name;
  }

  void set_meminfo_file_tmp(string f_name){
    meminfo=f_name;
  }

  void updata(){
    char cptmp[1000];
    sprintf(cptmp,"cp %s ./%s",proc_meminfo.c_str(),meminfo.c_str());
    system(cptmp);

    ifstream i_info(meminfo.c_str());
    string name,kb;
    i_info >> name >> memory_total >> kb;
    i_info >> name >> memory_free >> kb;
    i_info >> name >> buffers >> kb;
    i_info >> name >> cached >> kb;
    i_info >> name >> swap_cached >> kb;
    i_info >> name >> active >> kb;
    i_info >> name >> inactive >> kb;
    i_info >> name >> high_total >> kb;
    i_info >> name >> high_free >> kb;
    i_info >> name >> low_total >> kb;
    i_info >> name >> low_free >> kb;
    i_info >> name >> swap_total >> kb;
    i_info >> name >> swap_free >> kb;
    i_info >> name >> dirty >> kb;
    i_info >> name >> write_back >> kb;
    i_info >> name >> anon_pages >> kb;
    i_info >> name >> mapped >> kb;
    i_info >> name >> slab >> kb;
    i_info >> name >> s_reclaimable >> kb;
    i_info >> name >> s_unreclaim >> kb;
    i_info >> name >> page_tables >> kb;
    i_info >> name >> nfs_unstable >> kb;
    i_info >> name >> bounce >> kb;
    i_info >> name >> commit_limit >> kb;
    i_info >> name >> committed_as >> kb;
    i_info >> name >> v_malloc_total >> kb;
    i_info >> name >> v_malloc_used >> kb;
    i_info >> name >> v_malloc_chunk >> kb;
    i_info.close();

    char rm[1000];
    sprintf(rm,"chmod 777 %s",meminfo.c_str());
    system(rm);
    sprintf(rm,"rm %s",meminfo.c_str());
    system(rm);
  }

  int get_memory_total(){
    return memory_total;
  }

  int get_memory_free(){
    return memory_free;
  }

  int get_buffers(){
    return buffers;
  }

  int get_cached(){
    return cached;
  }

  int get_swap_cached(){
    return swap_cached;
  }

  int get_active(){
    return active;
  }

  int get_inactive(){
    return inactive;
  }

  int get_high_total(){
    return high_total;
  }

  int get_high_free(){
    return high_free;
  }

  int get_low_total(){
    return low_total;
  }

  int get_low_free(){
    return low_free;
  }

  int get_swap_total(){
    return low_free;
  }

  int get_swap_free(){
    return swap_free;
  }

  int get_dirty(){
    return dirty;
  }

  int get_write_back(){
    return write_back;
  }

  int get_anon_pages(){
    return anon_pages;
  }

  int get_mapped(){
    return mapped;
  }

  int get_slab(){
    return slab;
  }

  int get_s_reclaimable(){
    return s_reclaimable;
  }

  int get_s_unreclaim(){
    return s_unreclaim;
  }

  int get_page_tables(){
    return page_tables;
  }

  int get_nfs_unstable(){
    return nfs_unstable;
  }

  int get_bounce(){
    return bounce;
  }

  int get_commit_limit(){
    return commit_limit;
  }

  int get_committed_as(){
    return committed_as;
  }

  int get_v_malloc_total(){
    return committed_as;
  }

  int get_v_malloc_used(){
    return committed_as;
  }

  int get_v_malloc_chunk(){
    return v_malloc_chunk;
  }


private:
  string proc_meminfo;
  string meminfo;
  
  // kB
  int memory_total;
  int memory_free;
  int buffers;
  int cached;
  int swap_cached;
  int active;
  int inactive;
  int high_total;
  int high_free;
  int low_total;
  int low_free;
  int swap_total;
  int swap_free;
  int dirty;
  int write_back;
  int anon_pages;
  int mapped;
  int slab;
  int s_reclaimable;
  int s_unreclaim;
  int page_tables;
  int nfs_unstable;
  int bounce;
  int commit_limit;
  int committed_as;
  int v_malloc_total;
  int v_malloc_used;
  int v_malloc_chunk;
};

#endif
