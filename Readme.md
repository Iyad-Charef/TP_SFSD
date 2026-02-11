# File Partitioning by Hashing System

SFSD Practical Work (TP) – ESI Algiers  
By: **Charef Iyad (G11)** & **Amer el Khedoud Youcef (G11)**

---

##  Overview
This project implements a file partitioning system using **uniform hashing**.  
It demonstrates how to split a source file into multiple fragments, manage records, and analyze storage statistics.  
The program is menu-driven and interactive, making it easy to test different configurations.

---

##  Features
- Create sample source files with test records
- Partition files into **K fragments** using **M buffers**
- Search, insert, and delete records by key
- Display specific fragments or all fragments
- Show statistics (blocks, insertions, deletions, active records)
- Configure parameters: number of fragments (K) and buffers (M)

---

##  Usage
1. Compile with the provided `Makefile`:
   ```bash
   make
   ```

2. Run the program:

  ```bash 
  ./partition_program
  ```

3. Use the interactive menu to:
  Create sample files
  Partition by hashing
  Manage records
  Display fragments and statistics
  Adjust configuration (K and M)

## 🗒️ Notes
  Records have fields: key, name, age
  Fragment files are named fragment_X.dat
  Buffers and fragments can be reconfigured (must satisfy 2 < M < K)
  Data persists in .dat files across runs
