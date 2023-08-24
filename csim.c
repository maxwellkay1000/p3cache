#include "cachelab.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#define MAX_LINE_LENGTH 200
struct line {
  char valid_bit;
  long tag;
  int age;
};
typedef struct line line;
int hits, misses, evictions = 0;
int main(int argc, char *argv[]){
  //parse command line arguments
  int opt;
  int num_set_index_bits,
      num_lines_per_set,
      num_block_bits;
  char* filename;
  while((opt = getopt(argc, argv, "s:E:b:t:")) != -1){
      switch(opt){
          case 's':
              num_set_index_bits = atoi(optarg);
              break;
          case 'E':
              num_lines_per_set = atoi(optarg);
              break;
          case 'b':
              num_block_bits = atoi(optarg);
              break;
          case 't':
              filename = optarg;
              break;
          case '?':
              printf("unknown option\n");
              break;
      }
  }
  int num_sets = 2 << num_set_index_bits;
  line** cache = (line**)malloc(num_sets * sizeof(line*));
  for(unsigned int i = 0; i < num_sets; i++){
      cache[i] = (line*)malloc(num_lines_per_set * sizeof(line));
      for(unsigned int j = 0; j < num_lines_per_set; j++){
          cache[i][j].valid_bit = 0;
          cache[i][j].tag = 0;
          cache[i][j].age = 0;
      }
  }
  FILE *fp = NULL;
  fp = fopen(filename, "r");
  if (fp == NULL){
      fprintf(stderr, "File pointer is null!\n");
      exit(1);
  }
  char line[MAX_LINE_LENGTH];
  while(fgets(line, MAX_LINE_LENGTH, fp)){
      char operation;
      unsigned long address;
      char first;
      sscanf(line, "%c", &first);
      if(first == 'I'){
           continue;
      }
      sscanf(line, " %c %lx", &operation, &address);
      unsigned long tag_length = ((8 * sizeof(long)) - num_set_index_bits - num_block_bits);
      unsigned long set_index = ((address << (tag_length)) >> (tag_length + num_block_bits));
      unsigned long tag = address >> (num_block_bits + num_set_index_bits);
      int no_hit = 1;
      for(int i = 0; i < num_lines_per_set; i++){
          if ((cache[set_index][i].valid_bit == 1)&&
              (cache[set_index][i].tag == tag)){
              no_hit = 0;
              hits++;
              cache[set_index][i].age = 0; 
              if(operation == 'M'){
                hits++; 
              }
              break;
          }
      }
    if(no_hit){
        misses++;
        int i = 0;
        int is_full = 1; 
        while(i < num_lines_per_set){
            if(cache[set_index][i].valid_bit == 0){
                cache[set_index][i].valid_bit = 1;
                cache[set_index][i].tag = tag;
                cache[set_index][i].age = 0;
                is_full = 0; 
                if(operation == 'M'){
                    hits++; 
                }
                break;
            }
            i++; 
        }
        if(is_full){
            int greatest_age = 0; 
            int oldest_index = 0;
            for(int i = 0; i < num_lines_per_set; i++){
                if(cache[set_index][i].age > greatest_age){
                    greatest_age = cache[set_index][i].age; 
                    oldest_index = i; 
                }
            }
            cache[set_index][oldest_index].tag = tag; 
            cache[set_index][oldest_index].age = 0;//fix
            evictions++;  
            if(operation == 'M'){
                hits++; 
            }
        }
    }
    for(int i = 0; i < num_lines_per_set; i++){
        if(cache[set_index][i].valid_bit == 1){
            cache[set_index][i].age++; 
        }
    } 
  }
  fclose(fp);
   printSummary(hits, misses, evictions); //
  return 0;
}





