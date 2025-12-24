#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "functions.h"

//------------------------
// Global Constants
//------------------------
int K_FRAGMENTS = 5;
int M_BUFFERS = 3;

//------------------------
// Machine abstraite Tnof
//------------------------
void TnOF_open(t_TnOF **F, char *fname, char mode)
{
    *F = malloc(sizeof(t_TnOF));
    if (*F == NULL) {
        perror("TnOF_open: malloc failed");
        exit(EXIT_FAILURE);
    }
    //open existing file for reading/writing
    if (mode == 'E' || mode == 'e') {
        (*F)->f = fopen(fname, "rb+");
        if ((*F)->f == NULL) {
            perror("TnOF_open");
            exit(EXIT_FAILURE);
        }
        fread(&((*F)->h), sizeof(t_header), 1, (*F)->f);
    }
    //create new file
    else {
        (*F)->f = fopen(fname, "wb+");
        if ((*F)->f == NULL) {
            perror("TnOF_open");
            exit(EXIT_FAILURE);
        }
        //initialize header
        (*F)->h.nBlock = 0;
        (*F)->h.nIns = 0;
        (*F)->h.nDel = 0;
        fwrite(&((*F)->h), sizeof(t_header), 1, (*F)->f);
    }
}

void TnOF_close(t_TnOF *F)
{
    if (F == NULL) return;
    
    fseek(F->f, 0L, SEEK_SET);
    fwrite(&F->h, sizeof(t_header), 1, F->f);
    fclose(F->f);
    free(F);
}

void TnOF_readBlock(t_TnOF *F, long i, t_block *buf)
{
    fseek(F->f, sizeof(t_header) + (i - 1) * sizeof(t_block), SEEK_SET);
    fread(buf, sizeof(t_block), 1, F->f);
}

void TnOF_writeBlock(t_TnOF *F, long i, t_block *buf)
{
    fseek(F->f, sizeof(t_header) + (i - 1) * sizeof(t_block), SEEK_SET);
    fwrite(buf, sizeof(t_block), 1, F->f);
}

void setHeader(t_TnOF *F, char *hname, long val)
{
    if (strcmp(hname, "nBlock") == 0) { F->h.nBlock = val; return; }
    if (strcmp(hname, "nIns") == 0) { F->h.nIns = val; return; }
    if (strcmp(hname, "nDel") == 0) { F->h.nDel = val; return; }
    fprintf(stderr, "setHeader: Unknown headerName: \"%s\"\n", hname);
}

long getHeader(t_TnOF *F, char *hname)
{
    if (strcmp(hname, "nBlock") == 0) return F->h.nBlock;
    if (strcmp(hname, "nIns") == 0) return F->h.nIns;
    if (strcmp(hname, "nDel") == 0) return F->h.nDel;
    fprintf(stderr, "getHeader: Unknown headerName: \"%s\"\n", hname);
    return -1;
}

//----------------------
// uniform hashing function
//-------------------

int hash_function(char *key, int K)
{
    unsigned long hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = hash * 31 + key[i];
    }
    return hash % K;
    // returns the Asci sum of KEY Mod K
}

// simplify manipulating multiple filenames
void get_fragment_filename(int fragment_num, char *filename)
{
    sprintf(filename, "fragment_%d.dat", fragment_num);
}


//----------------------------------------------------------------------------------------
// Partioning function with multiple passes using M buffers and a K uniforme hashing function 
//------------------------------------------------------------------------------------------

void partition_file_by_hashing(char *source_filename, int K, int M)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║      FILE PARTITIONING BY HASHING                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Source file: %s\n", source_filename);
    printf("Number of fragments (K): %d\n", K);
    printf("Number of buffers (M): %d\n\n", M);
    
    // Opening File, header

    t_TnOF *F;
    TnOF_open(&F, source_filename, 'E');
    
    long N = getHeader(F, "nBlock");
    printf("File F contains: %ld blocks\n\n", N);
    
    // allocating M buffers
    t_block *T = malloc(M * sizeof(t_block));
    if (T == NULL) {
        printf("Error: Memory allocation failed!\n");
        TnOF_close(F);
        return;
    }
    
    t_TnOF **g = malloc((M - 1) * sizeof(t_TnOF*));
    long *numB = malloc((M - 1) * sizeof(long));
    
    //current hash range
    int bi = 0;
    int bs = M - 1;

    // Formula: ceil(K/(M-1))
    int num_passes = (int)ceil((double)K / (double)(M - 1));
    
    printf("┌─────────────────────────────────────────────────────────┐\n");
    printf("│ ALGORITHM PARAMETERS:                                   │\n");
    printf("│  - Buffers for fragments: %d (T[0] to T[%d])            │\n", 
           M - 1, M - 2);
    printf("│  - Buffer for reading F: 1 (T[%d])                      │\n", M - 1);
    printf("│  - Number of passes: %d                                 │\n", num_passes);
    printf("└─────────────────────────────────────────────────────────┘\n\n");
    
    int pass_number = 1;
    
    // main loop 
    while (bi < K) {
        printf("╔════════════════════════════════════════════════════════════╗\n");
        printf("║  PASS %d/%d (bi=%d, bs=%d)                                 ║\n", pass_number, num_passes, bi, bs);
        printf("╚════════════════════════════════════════════════════════════╝\n");
        
        // Open fragment files for current pass range
        printf("openin fragments: ");
        for (int x = bi; x < bs && x < K; x++) {
            char fname[50];
            get_fragment_filename(x, fname);
            
            TnOF_open(&g[x - bi], fname, 'N');
            numB[x - bi] = 0;
            T[x - bi].nb = 0;
            for (int j = 0; j < MAXTAB; j++) {
                T[x - bi].del[j] = ' ';
            }
            
            printf("F%d ", x);
        }
        printf("\n");
        
        // Read source file and distribute records to fragments
        printf("Scanning all %ld blocks of F...\n", N);
        
        int records_distributed = 0;
        
        for (long i = 1; i <= N; i++) {
            TnOF_readBlock(F, i, &T[M - 1]);
            
            // Process each record in block
            for (int j = 0; j < T[M - 1].nb; j++) {
            // skip deleted records
            if (T[M - 1].del[j] == '*') continue;
            
            // get record hash
            t_rec record = T[M - 1].tab[j];
            int x_prime = hash_function(record.key, K);
            
            // Check if record belongs to current pass range
            if (x_prime >= bi && x_prime < bs && x_prime < K) {
                int x = x_prime - bi;
                
                if (T[x].nb < MAXTAB) {
                // Add to buffer
                T[x].tab[T[x].nb] = record;
                T[x].del[T[x].nb] = ' ';
                T[x].nb++;
                records_distributed++;
                } else {
                // Buffer full
                numB[x]++;
                TnOF_writeBlock(g[x], numB[x], &T[x]);
                
                // new buffer 
                T[x].tab[0] = record;
                T[x].del[0] = ' ';
                T[x].nb = 1;
                records_distributed++;
                }
            }
            }
        }
        
        // Writing remaining buffers
        for (int x = bi; x < bs && x < K; x++) {
            int buffer_index = x - bi;
            
            if (T[buffer_index].nb > 0) {
                numB[buffer_index]++;
                TnOF_writeBlock(g[buffer_index], numB[buffer_index], &T[buffer_index]);
            }
            
            setHeader(g[buffer_index], "nBlock", numB[buffer_index]);
            
            // total record count
            long total_records = 0;
            if (numB[buffer_index] > 0) {
                for (long blk = 1; blk <= numB[buffer_index]; blk++) {
                    t_block temp_buf;
                    TnOF_readBlock(g[buffer_index], blk, &temp_buf);
                    total_records += temp_buf.nb;
                }
            }
        
            setHeader(g[buffer_index], "nIns", total_records);
            
            TnOF_close(g[buffer_index]);
        }
        
        printf("✓ Pass %d complete: %d records distributed\n\n", pass_number, records_distributed);
        
        // next fragments range
        bi = bs;
        bs = bs + M - 1;
        if (bs > K) bs = K;
    
        pass_number++;
    }
    
    //finish
    TnOF_close(F);
    free(T);
    free(g);
    free(numB);
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  PARTITIONING COMPLETE!                                    ║\n");
    printf("║  File F successfully fragmented into %d fragments          ║\n", K);
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("┌─────────────────────────────────────────────────────────┐\n");
    printf("│ COST ANALYSIS :                                         │\n");
    printf("│  - Number of passes: K/(M-1) = %d/%d ≈ %d              │\n", 
           K, M - 1, num_passes);
    printf("│  - Total reads: K*N/(M-1) = %d*%ld/%d = %ld            │\n", 
           K, N, M - 1, (K * N) / (M - 1));
    printf("│  - Total writes: N = %ld                                │\n", N);
    printf("│  - Total I/O operations: %ld                            │\n", 
           (K * N) / (M - 1) + N);
    printf("└─────────────────────────────────────────────────────────┘\n\n");
}

int search_in_fragments(char *key, int K, int *fragment_num, long *block_num, int *position, t_rec *record)
{
    // determine fragment using hash function
    *fragment_num = hash_function(key, K);
    
    char fname[50];
    get_fragment_filename(*fragment_num, fname);
    
    t_TnOF *fragment_file;
    TnOF_open(&fragment_file, fname, 'E');
    
    long N = getHeader(fragment_file, "nBlock");
    t_block buf;
    // Sequential search through all blocks in fragment
    for (long i = 1; i <= N; i++) {
        TnOF_readBlock(fragment_file, i, &buf);
        // search through records
        for (int j = 0; j < buf.nb; j++) {
            if (buf.del[j] != '*' && strcmp(buf.tab[j].key, key) == 0) {
                *block_num = i;
                *position = j;
                *record = buf.tab[j];
                TnOF_close(fragment_file);
                return 1; // record found
            }
        }
    }
    
    TnOF_close(fragment_file);
    return 0;
}

int insert_into_fragments(t_rec record, int K)
{
    int frag_num;
    long block_num;
    int pos;
    t_rec existing_rec;
    // check if record already exists 
    if (search_in_fragments(record.key, K, &frag_num, &block_num, &pos, &existing_rec)) {
        printf("Error: Key '%s' already exists in fragment %d\n", record.key, frag_num);
        return 0;
    }
    // determine target fragment using hash function
    int fragment_num = hash_function(record.key, K);
    
    char fname[50];
    get_fragment_filename(fragment_num, fname);
    
    t_TnOF *fragment_file;
    // open fragment file for writing
    TnOF_open(&fragment_file, fname, 'E');
    
    long N = getHeader(fragment_file, "nBlock");
    t_block buf;
    // try to insert in last block if it's possible
    if (N > 0) {
        TnOF_readBlock(fragment_file, N, &buf);
        
        if (buf.nb < MAXTAB) {
            buf.tab[buf.nb] = record;
            buf.del[buf.nb] = ' ';
            buf.nb++;
            TnOF_writeBlock(fragment_file, N, &buf);
            long nIns = getHeader(fragment_file, "nIns");
            setHeader(fragment_file, "nIns", nIns + 1);
            TnOF_close(fragment_file);
            printf("Record with key '%s' inserted in fragment %d, block %ld\n", record.key, fragment_num, N);
            return 1; // inserted
        }
    }
    // create new block 
    buf.nb = 1;
    buf.tab[0] = record;
    buf.del[0] = ' ';
    for (int i = 1; i < MAXTAB; i++) {
        buf.del[i] = ' ';
    }
    // write new block
    long new_block = N + 1;
    TnOF_writeBlock(fragment_file, new_block, &buf);
    // upate headers
    setHeader(fragment_file, "nBlock", new_block);
    long nIns = getHeader(fragment_file, "nIns");
    setHeader(fragment_file, "nIns", nIns + 1);
    
    TnOF_close(fragment_file);
    printf("Record with key '%s' inserted in fragment %d, block %ld (new block)\n", record.key, fragment_num, new_block);
    return 1;
}

int delete_from_fragments(char *key, int K)
{
    int fragment_num;
    long block_num;
    int position;
    t_rec record;
    // find record first
    if (!search_in_fragments(key, K, &fragment_num, &block_num, &position, &record)) {
        printf("Error: Key '%s' not found\n", key);
        return 0;
    }
    
    char fname[50];
    get_fragment_filename(fragment_num, fname);
    // open fragment file for writing
    t_TnOF *fragment_file;
    TnOF_open(&fragment_file, fname, 'E');
    // read block containing the record
    t_block buf;
    TnOF_readBlock(fragment_file, block_num, &buf);
    // logically deleting record
    buf.del[position] = '*';
    // write updated block
    TnOF_writeBlock(fragment_file, block_num, &buf);
    // update header
    long nDel = getHeader(fragment_file, "nDel");
    setHeader(fragment_file, "nDel", nDel + 1);
    
    TnOF_close(fragment_file);
    printf("Record with key '%s' deleted from fragment %d, block %ld, position %d\n", 
           key, fragment_num, block_num, position);
    return 1;
}

//-------------------------------------------------------------------
// DISPLAY FUNCTIONS
//-------------------------------------------------------------------
void display_fragment(int fragment_num)
{
    char fname[50];
    get_fragment_filename(fragment_num, fname);
    
    t_TnOF *fragment_file;
    TnOF_open(&fragment_file, fname, 'E');
    
    long N = getHeader(fragment_file, "nBlock");
    long nIns = getHeader(fragment_file, "nIns");
    long nDel = getHeader(fragment_file, "nDel");
    
    printf("\n--- Fragment %d (%s) ---\n", fragment_num, fname);
    printf("Blocks: %ld, Insertions: %ld, Deletions: %ld\n", N, nIns, nDel);
    printf("%-20s %-30s %-10s\n", "Key", "Name", "Age");
    printf("------------------------------------------------------------\n");
    
    t_block buf;
    for (long i = 1; i <= N; i++) {
        TnOF_readBlock(fragment_file, i, &buf);
        
        for (int j = 0; j < buf.nb; j++) {
            if (buf.del[j] != '*') {
                printf("%-20s %-30s %-10d\n", 
                       buf.tab[j].key, 
                       buf.tab[j].name, 
                       buf.tab[j].age);
            }
        }
    }
    
    TnOF_close(fragment_file);
}

void display_all_fragments(int K)
{
    printf("\n========== All Fragments ==========\n");
    for (int i = 0; i < K; i++) {
        display_fragment(i);
    }
    printf("===================================\n");
}

void display_statistics(int K)
{
    printf("\n========== Statistics ==========\n");
    printf("%-12s %-10s %-12s %-12s %-15s\n", "Fragment", "Blocks", "Insertions", "Deletions", "Active Records");
    printf("----------------------------------------------------------------\n");
    
    long total_blocks = 0, total_ins = 0, total_del = 0;
    
    for (int i = 0; i < K; i++) {
        char fname[50];
        get_fragment_filename(i, fname);
        
        t_TnOF *fragment_file;
        TnOF_open(&fragment_file, fname, 'E');
        
        long N = getHeader(fragment_file, "nBlock");
        long nIns = getHeader(fragment_file, "nIns");
        long nDel = getHeader(fragment_file, "nDel");
        
        printf("%-12d %-10ld %-12ld %-12ld %-15ld\n", i, N, nIns, nDel, nIns - nDel);
        
        total_blocks += N;
        total_ins += nIns;
        total_del += nDel;
        
        TnOF_close(fragment_file);
    }
    
    printf("----------------------------------------------------------------\n");
    printf("%-12s %-10ld %-12ld %-12ld %-15ld\n", "TOTAL", total_blocks, total_ins, total_del, total_ins - total_del);
    printf("================================\n\n");
}

//-------------------------------------------------------------------
// SAMPLE FILE CREATION
//-------------------------------------------------------------------
void create_sample_file(char *filename, int num_records)
{
    t_TnOF *file;
    TnOF_open(&file, filename, 'N');
    
    t_block buf;
    buf.nb = 0;
    
    printf("Creating sample file '%s' with %d records...\n", filename, num_records);
    // generate test keys
    for (int i = 0; i < num_records; i++) {
        t_rec rec;
        sprintf(rec.key, "KEY%04d", i);
        sprintf(rec.name, "Student %d", i);
        rec.age = 18 + (i % 10);
        // fill the buffer
        if (buf.nb < MAXTAB) {
            buf.tab[buf.nb] = rec;
            buf.del[buf.nb] = ' ';
            buf.nb++;
        }// write block to file
         else {
            long next_block = getHeader(file, "nBlock") + 1;
            TnOF_writeBlock(file, next_block, &buf);
            setHeader(file, "nBlock", next_block);
            long nIns = getHeader(file, "nIns");
            setHeader(file, "nIns", nIns + buf.nb);
            
            buf.nb = 1;
            buf.tab[0] = rec;
            buf.del[0] = ' ';
        }
    }
    // write remaining records in buffer
    if (buf.nb > 0) {
        long next_block = getHeader(file, "nBlock") + 1;
        TnOF_writeBlock(file, next_block, &buf);
        setHeader(file, "nBlock", next_block);
        long nIns = getHeader(file, "nIns");
        setHeader(file, "nIns", nIns + buf.nb);
    }
    
    TnOF_close(file);
    printf("Sample file created successfully!\n\n");

}


