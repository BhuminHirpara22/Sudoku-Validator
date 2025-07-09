#include <bits/stdc++.h>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
using namespace std;
using namespace chrono;

vector<vector<int>> sudoku;
int n;
int k;
int taskInc;
FILE *output_file;

// Function to check all the rows of sudoku
bool checkRows(){
    bool res = true;
    for (int i = 0; i < n; i++){
        bool seen[n+1];
        bool temp = true;
        for (int j = 0; j <= n; j++) seen[j] = false; // Initialize seen array
        for (int j = 0; j < n; j++){
            int val = sudoku[i][j];
            if (seen[val]) temp = false; // Duplicate found
            seen[val] = true;
        }
        fprintf(output_file, "Row %d is %s\n", i+1, temp ? "valid" : "invalid");
        if (!temp) return false;  // Early exit if invalid row found
        res = res && temp;
    }
    return res;
}

// Function to check all the columns of sudoku
bool checkColumns(){
    bool res = true;
    for (int i = 0; i < n; i++){
        bool seen[n+1];
        bool temp = true;
        for (int j = 0; j <= n; j++) seen[j] = false; // Initialize seen array
        for (int j = 0; j < n; j++){
            int val = sudoku[j][i];
            if (seen[val]) temp = false;
            seen[val] = true;
        }
        fprintf(output_file, "Column %d is %s\n", i+1, temp ? "valid" : "invalid");
        if (!temp) return false;  // Early exit if invalid column found
        res = res && temp;
    }
    return res;
}

// Function to check all the sub-grids of sudoku
bool checkSubGrids(){
    bool res = true;
    int sn = sqrt(n); // Size of subgrid
    for (int g = 0; g < n; g++){
        int rowS = (g / sn) * sn;
        int colS = (g % sn) * sn;
        bool seen[n+1];
        bool temp = true;
        for (int j = 0; j <= n; j++) seen[j] = false; // Initialize seen array
        for (int i = 0; i < sn; i++){
            for (int j = 0; j < sn; j++){
                int val = sudoku[rowS + i][colS + j];
                if (seen[val]) temp = false;
                seen[val] = true;
            }
        }
        fprintf(output_file, "Grid %d is %s\n", g+1, temp ? "valid" : "invalid");
        if (!temp) return false;  // Early exit if invalid subgrid found
        res = res && temp;
    }
    return res;
}

// Function for reading the input file
void readFile(){
    ifstream inputFile("input.txt");
    inputFile >> k >> n >> taskInc;
    sudoku.resize(n, vector<int>(n));
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            inputFile >> sudoku[i][j];
        }
    }
    inputFile.close();
}

int main(){
    readFile();
    // Open output file once using FILE pointer
    output_file = fopen("output.txt", "w");
    auto sTime = system_clock::now(); // Start time
    
    // Validate the sudoku by checking rows, columns, and sub-grids
    bool res = checkRows() && checkColumns() && checkSubGrids();
    
    auto eTime = system_clock::now(); // End time
    long long totalTime = duration_cast<microseconds>(eTime - sTime).count(); // Total time in microseconds
    
    // Print final log messages
    if (res) {
        fprintf(output_file, "Sudoku is valid.\n");
    } else {
        fprintf(output_file, "Sudoku is invalid.\n");
    }
    fprintf(output_file, "Time taken is %lld microseconds.\n", totalTime);
    
    fclose(output_file);
    return 0;
}
