#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define MAXTAB 10
#define MAX_KEY_LEN 20

// record struct
typedef struct {
    char key[MAX_KEY_LEN];
    char name[50];
    int age;
} t_rec;

//block struct
typedef struct blck {
    t_rec tab[MAXTAB];
    char del[MAXTAB];
    int nb;
} t_block;

// file header
typedef struct hdr {
    long nBlock;
    long nIns;
    long nDel;
} t_header;

// tnof file struct
typedef struct TnOFstr {
    FILE *f;
    t_header h;
} t_TnOF;

//machine abstraite functions
void TnOF_open(t_TnOF **F, char *fname, char mode);
void TnOF_close(t_TnOF *F);
void TnOF_readBlock(t_TnOF *F, long i, t_block *buf);
void TnOF_writeBlock(t_TnOF *F, long i, t_block *buf);
void setHeader(t_TnOF *F, char *hname, long val);
long getHeader(t_TnOF *F, char *hname);

extern int K_FRAGMENTS;
extern int M_BUFFERS;
//TP functions
int hash_function(char *key, int K);
void partition_file_by_hashing(char *source_filename, int K, int M);
int search_in_fragments(char *key, int K, int *fragment_num, long *block_num, int *position, t_rec *record);
int insert_into_fragments(t_rec record, int K);
int delete_from_fragments(char *key, int K);

// helper functions
void get_fragment_filename(int fragment_num, char *filename);
void display_fragment(int fragment_num);
void display_all_fragments(int K);
void create_sample_file(char *filename, int num_records);
void display_statistics(int K);

#endif
