#include<bits/stdc++.h>
using namespace std;
using namespace chrono;

vector<vector<int>> sudoku; // Sudoku grid
int n; // Sudoku size
int k; // Number of threads
int taskInc; // Size of tasks perfromed by each thread at once
int C = 0; // Overall task grabbed
atomic<bool> Lock(false);  // CAS-based lock using atomic<bool> 
bool isValid = true; // Validity of the grid

// Struct for storing thread data
typedef struct {
    int id;
} Data;

// Struct for storing Logs
typedef struct {
    system_clock::time_point timestamp;
    string message;
} Log;

vector<vector<Log>> logs;// Vector for storing logs of each thread seperately to avoid race condition
vector<pair<long long,long long>> inTimes; // Vector of pairs in which first argument is the summation of all the times taken to enter and second argument contains the max of it for each thread
vector<pair<long long,long long>> exTimes; // Vector of pairs in which first argument is the summation of all the times taken to exit and second argument contains the max of it for each thread

// Function to check the row of the soduko
bool checkRow(int index){
    vector<bool> seen(n+1); 
    for(int j=0;j<=n;j++) seen[j] = false;
    for(int j=0;j<n;j++){
        int val = sudoku[index][j];
        if(seen[val]) return false; // If already seen element repeats then its an invalid sudoku
        seen[val] = true; // Marks the element seen
    }
    return true;
}

// Function to check the column of the soduko
bool checkColumn(int index){
    vector<bool> seen(n+1);
    for(int j=0;j<=n;j++) seen[j] = false;
    for(int j=0;j<n;j++){
        int val = sudoku[j][index];
        if(seen[val]) return false; // If already seen element repeats then its an invalid sudoku
        seen[val] = true; // Marks the element seen
    }
    return true;
}

// Function to check the sub-grid of the soduko
bool checkSubGrid(int index){
    int sn = (int)pow(n,0.5);
    int rowS = (index/sn)*sn;
    int colS = (index%sn)*sn;
    vector<bool> seen(n+1);
    for(int j=0;j<=n;j++) seen[j] = false;
    for(int k=0;k<sn;k++){
        for(int j=0;j<sn;j++){
            int val = sudoku[rowS+k][colS+j];
            if(seen[val]) return false; // If already seen element repeats then its an invalid sudoku
            seen[val] = true; // Marks the element seen
        }
    }
    return true;
}

// Function to convert time to string for updating logs
string timeTOString(system_clock::time_point time){
    // Convert to time_t (seconds since epoch)
    time_t tt = system_clock::to_time_t(time);

    // Convert to local time
    tm local_tm = *localtime(&tt);

    // Extract microseconds
    auto duration = time.time_since_epoch();
    auto microsec = duration_cast<microseconds>(duration) % 1'000'000;

    // Format time as string with microseconds
    stringstream ss;
    ss << put_time(&local_tm, "%H:%M:%S") << "." 
       << setfill('0') << setw(6) << microsec.count(); // Zero-padded microseconds

    return ss.str();
}

// Function to process tasks assigned to a thread
void processTask(int oldC,int newC,int threadId){
    for(int task=oldC;task<newC;task++){
        if(!isValid) break; // condition for early termination
        bool valid = false; // Current status of the operation
        // Dividing task into 3 sub-groups based on its index
        if(task<n){
            valid = checkRow(task);
            if(!isValid) break; // Early termination
            // Updating logs
            logs[threadId].push_back({system_clock::now(),"Thread " + to_string(threadId+1) + " completes checking of row " + to_string(task+1) + " at " + timeTOString(system_clock::now()) + " and finds it as " + (valid ? "valid " : "invalid ")});
        }else if(task<2*n){
            valid = checkColumn(task-n);
            if(!isValid) break; // Early termination
            // Updating logs
            logs[threadId].push_back({system_clock::now(),"Thread " + to_string(threadId+1) + " completes checking of column " + to_string(task+1-n) + " at " + timeTOString(system_clock::now()) + " and finds it as " + (valid ? "valid " : "invalid ")});
        }else{
            valid = checkSubGrid(task-2*n);
            if(!isValid) break; // Early termination
            // Updating logs
            logs[threadId].push_back({system_clock::now(),"Thread " + to_string(threadId+1) + " completes checking of sub-grid " + to_string(task+1-2*n) + " at "  + timeTOString(system_clock::now()) + " and finds it as " + (valid ? "valid " : "invalid ")});
        }
        // Update sudoku status
        if(!valid){
            isValid = false;
            break;
        }
    }
}

// Function to assign tasks to threads using critical section for race conditions
void* getTask(void* arg){
    int totalTasks = 3*n; // Total tasks
    Data* data = (Data*)arg; // Extracting data from the argument
    while(isValid){
        int oldC,newC,actualTaskInc;
        auto rTime = system_clock::now(); // Critical section request time
        // Updating logs
        logs[data->id].push_back({rTime,"Thread " + to_string(data->id+1) + " requests to enter CS1 at " + timeTOString(rTime)});
        // CAS-based lock acquisition
        bool expected = false;
        // Using compare and swap to allow only one thread to enter the critical section
        while(!Lock.compare_exchange_strong(expected,true,memory_order_acquire)){
            expected = false; // reset expected for next attempt
        }
        // If grid is invalid then early termination
        if(!isValid){
            Lock.store(false,memory_order_release);  // Release lock using store
            break;
        }
        auto inTime = system_clock::now(); // Critical section entry time
        long long timeToEnter = duration_cast<microseconds>(inTime - rTime).count(); // Calculates time taken to enter the critical section
        // Updating logs
        logs[data->id].push_back({inTime,"Thread " + to_string(data->id+1) + " enters CS1 at " + timeTOString(inTime)});
        inTimes[data->id] = {inTimes[data->id].first+timeToEnter,max(timeToEnter,inTimes[data->id].second)}; // Updating the sum of entry time and max entry time for this thread
        oldC = C; // Updates oldC
        newC = min(oldC+taskInc,totalTasks); // prevents task from exceeding the limit
        actualTaskInc = newC-oldC; // Calculating the actual tasks size
        C = newC; // Updates current counter
        if(oldC>=totalTasks || actualTaskInc<=0){
            Lock.store(false,memory_order_release);  // Release lock using store
            auto exTime = system_clock::now(); // Calculates the exit time
            long long timeToExit = duration_cast<microseconds>(exTime - inTime).count(); // Calculates time taken to exit the cricital section
            // Updating logs
            logs[data->id].push_back({exTime,"Thread " + to_string(data->id+1) + " exits CS1 at " + timeTOString(exTime)});// Updating the sum of exit time and max exit time for this thread
            exTimes[data->id] = {exTimes[data->id].first+timeToExit,max(timeToExit,exTimes[data->id].second)};
            break;
        }
        for(int task=oldC;task<newC;task++){
            if(!isValid) break;// Early termination
            if(task<n){
                if(!isValid) break; // Early termination
                // Updating logs
                logs[data->id].push_back({system_clock::now(),"Thread " + to_string(data->id+1) + " grabs row " + to_string(task+1) + " at " + timeTOString(system_clock::now())});
            }else if(task<2*n){
                if(!isValid) break; // Early termination
                // Updating logs
                logs[data->id].push_back({system_clock::now(),"Thread " + to_string(data->id+1) + " grabs column " + to_string(task+1-n) + " at " + timeTOString(system_clock::now())});
            }else{
                if(!isValid) break; // Early termination
                // Updating logs
                logs[data->id].push_back({system_clock::now(),"Thread " + to_string(data->id+1) + " grabs sub-grid " + to_string(task+1-2*n) + " at " + timeTOString(system_clock::now())});
            }
        }
        // Early terminatiion
        if(!isValid){
            Lock.store(false,memory_order_release);  // Release lock using store
            auto exTime = system_clock::now(); // Calculates the exit time
            long long timeToExit = duration_cast<microseconds>(exTime - inTime).count(); // Calculates time taken to exit the cricital section
            exTimes[data->id] = {exTimes[data->id].first+timeToExit,max(timeToExit,exTimes[data->id].second)};
            break;
        }
        Lock.store(false,memory_order_release);  // Release lock using store
        auto exTime = system_clock::now(); // Calculates the exit time
        long long timeToExit = duration_cast<microseconds>(exTime - inTime).count(); // Calculates time taken to exit the cricital section
        // Updating logs
        logs[data->id].push_back({exTime,"Thread " + to_string(data->id+1) + " exits CS1 at " + timeTOString(exTime)});// Updating the sum of exit time and max exit time for this thread
        exTimes[data->id] = {exTimes[data->id].first+timeToExit,max(timeToExit,exTimes[data->id].second)};
        
        processTask(oldC,newC,data->id);
    }
    pthread_exit(NULL);
}

// Function to read the input file
void readFile(){
    ifstream inputFile("input.txt"); // Opens the input file
    inputFile >> k >> n >> taskInc; 
    sudoku.resize(n,vector<int>(n)); // Resize sudoku grid based on the input
    logs.resize(k); // Resize logs to the number of threads
    inTimes.resize(k,{0,0}); // Resize entry-times based on the number of threads
    exTimes.resize(k,{0,0}); // Resize exit-times based on the number of threads
    for (int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            inputFile >> sudoku[i][j]; // Takes input for each sudoku cell
        }
    }
    inputFile.close(); // Closes input file
}

// Function to compare logs based on timestamp
bool compare(Log& l1,Log& l2){
    return l1.timestamp<l2.timestamp;
}

int main(){
    readFile(); // Reads file
    vector<pthread_t> threads(k); // Vector to store threads
    vector<Data> data(k); // Vector to store thread Data
    auto sTime = system_clock::now(); // Start time
    for(int i=0;i<k;i++){
        data[i].id = i; // Assign ID to threads
        pthread_create(&threads[i],nullptr,getTask,&data[i]); // Creates threads and begins validation
    }
    for(int j=0;j<k;j++){
        pthread_join(threads[j],NULL); // Joins threads after succesful validation
    }
    auto eTime = system_clock::now(); // End time
    ofstream outputFile("output.txt"); // Opens output file
    vector<Log> allLogs; // Vector to store all the logs
    for(int i=0;i<k;i++){
        for(int j=0;j<logs[i].size();j++){
            allLogs.push_back(logs[i][j]); // Adds logs to each thread to the overall vector
        }
    }
    sort(allLogs.begin(),allLogs.end(),compare); // Sorts all the logs based on the timestamp
    for(int i = 0; i < allLogs.size(); i++) {
        outputFile << allLogs[i].message << endl; // Outputs log messages
    }    
    // Outputs the result
    outputFile << "Sudoku is" << (isValid ? " Valid" : " inValid") << endl;
    long long maxInTime=0,maxExTime=0,totalTime; 
    double avgInTime=0,avgExTime=0;
    totalTime = duration_cast<microseconds>(eTime-sTime).count(); // Calculates total time taken to validate the grid
    for(int i=0;i<k;i++){
        maxInTime = max(maxInTime,inTimes[i].second);// Max time is the max of max time taken by each thread to enter 
        avgInTime += inTimes[i].first;// First summing all the entry times
        maxExTime = max(maxExTime,exTimes[i].second);// Max time is the max of max time taken by each thread to exit
        avgExTime += exTimes[i].first; // First summing all the exit times
    }
    avgExTime /= (double)exTimes.size(); // Calculates average entry time
    avgInTime /= (double)inTimes.size(); // Calculates average exit time
    // Ouputs various time logs
    outputFile << "The total time taken is " << totalTime << " microseconds" << endl;
    outputFile << "Average time taken by a thread to enter the CS is " << avgInTime << " microseconds" << endl;
    outputFile << "Average time taken by a thread to exit the CS is " << avgExTime << " microseconds" << endl;  
    outputFile << "Worst-case time taken by a thread to enter the CS is " << maxInTime << " microseconds" << endl;
    outputFile << "Worst-case time taken by a thread to exit the CS is " << maxExTime << " microseconds" << endl;
    outputFile.close();
    return 0;
}
