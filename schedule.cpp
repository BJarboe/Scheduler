/** @name Scheduler
 *  @file schedule.cpp
 *  @brief Main file to emulate process scheduling with CPU and IO bursts using different strategies.
*/

#include "schedule.h"
#include "log.h"

using namespace std;

// Exit with failure and display an error message
void fail(const string& message) {
    cerr << message << endl;
    exit(EXIT_FAILURE);
}

// Display usage error and exit
void usage_error() {
    cerr << "\nUsage: ./schedule [Burst file]" << endl
         << "\tOptions:" << endl
         << "\t\t-s [Scheduling strategy]" << endl
         << "\t\t\tfcfs - First Come First Served," << endl
         << "\t\t\trr - Round Robin." << endl
         << "\t\t-q [Time Quantum]" << endl
         << "\t\t\tMust be larger than 0" << endl;
    exit(EXIT_FAILURE);
}

// Parse process bursts from the input file
vector<deque<Burst>> parse_process(char* filename) {
    vector<deque<Burst>> processes;
    fstream burst_stream;
    burst_stream.open(filename);

    if (!burst_stream.is_open()) {
        fail("Unable to open " + string(filename) + ". Verify file name and directory contents.");
    }

    string line;
    for (int i = 0; getline(burst_stream, line); i++) {
        deque<Burst> process;
        stringstream ss(line);
        int val;

        for (int j = 0; ss >> val; j++) {
            if (val <= 0) {
                fail("A burst number must be greater than 0.");
            }

            Burst burst;
            burst.id = i;
            burst.burst_type = (j % 2 == 1) ? IO : CPU; // Odd index -> IO burst
            burst.duration = val;

            if (ss.peek() == ',' || ss.peek() == ' ') {
                ss.ignore();
            }
            process.push_back(burst);
        }

        if (process.size() % 2 == 0) {
            fail("There must be an odd number of bursts for each process.");
        }

        processes.push_back(process);
    }

    burst_stream.close();

    if (processes.empty()) {
        fail("No bursts found. Verify " + string(filename) + "'s contents.");
    }

    return processes;
}

// Sort the blocked queue based on the shortest burst duration
void sortQ(deque<deque<Burst>>& blockedQ) {
    vector<deque<Burst>> tempVec(blockedQ.begin(), blockedQ.end());
    std::stable_sort(tempVec.begin(), tempVec.end(),
        [](const deque<Burst>& a, const deque<Burst>& b) -> bool {
            if (!a.empty() && !b.empty()) {
                return a.front().duration < b.front().duration;
            } else if (a.empty()) {
                cerr << "Compared burst deque is empty." << endl;
                return true;
            } else {
                cerr << "Compared burst deque is empty." << endl;
                return false;
            }
        }
    );
    copy(tempVec.begin(), tempVec.end(), blockedQ.begin());
}

// Parse command-line flags
void parse_flag(int argc, char *argv[], string& strategy, int& quantum) {
    int option;
    string arg;
    while ((option = getopt(argc, argv, "s:q:")) != -1) {
        switch (option) {
            case 's':
                arg = optarg;
                if (arg == "rr" || arg == "fcfs") {
                    strategy = arg;
                } else {
                    fail(optarg + string(" is an invalid scheduling strategy!"));
                }
                break;
            case 'q':
                if (stoi(optarg) > 0) {
                    quantum = stoi(optarg);
                } else {
                    fail("Time quantum must be a number greater than 0.");
                }
                break;
            default:
                usage_error();
        }
    }
}

// Round Robin scheduling implementation
void round_robin(int quantum, vector<deque<Burst>> processes) {
    deque<deque<Burst>> blockedQ;
    deque<deque<Burst>> readyQ(processes.begin(), processes.end());

    int ioExecutedTimes[processes.size()] = {0};
    int cpuExecutedTimes[processes.size()] = {0};

    int elapsed_time = 0;
    int total_time = 0;

    Burst* cpu_burst;
    Burst* io_burst;

    ExecutionStopReasonType stopReason;
    completionLog finished[processes.size()];
    int f = 0;

    while (!readyQ.empty() || !blockedQ.empty()) {
        stopReason = QUANTUM_EXPIRED;
        elapsed_time = quantum;

        if (readyQ.empty()) {
            io_burst = &(blockedQ.front().front());
            elapsed_time = io_burst->duration;

            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) {
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                } else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }

            for (int i = 0; i < blockedQ.size(); i++) {
                if (blockedQ.front().front().duration == 0) {
                    blockedQ.front().pop_front();
                    readyQ.push_back(blockedQ.front());
                    blockedQ.pop_front();
                }
            }

            total_time += elapsed_time;
            continue;
        }

        cpu_burst = &(readyQ.front().front());
        if (cpu_burst->duration <= elapsed_time) {
            cpuExecutedTimes[cpu_burst->id] += cpu_burst->duration;
            elapsed_time = cpu_burst->duration;
            cpu_burst->duration = 0;
            stopReason = (readyQ.front().size() != 1) ? ENTER_IO : COMPLETED;
        } else {
            cpuExecutedTimes[cpu_burst->id] += elapsed_time;
            cpu_burst->duration -= elapsed_time;
        }

        if (!blockedQ.empty()) {
            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) {
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                } else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }

            for (int i = 0; i < blockedQ.size(); i++) {
                if (blockedQ.front().front().duration == 0) {
                    blockedQ.front().pop_front();
                    readyQ.push_back(blockedQ.front());
                    blockedQ.pop_front();
                }
            }
        }

        total_time += elapsed_time;

        log_cpuburst_execution(cpu_burst->id, cpuExecutedTimes[cpu_burst->id], ioExecutedTimes[cpu_burst->id], total_time, stopReason);

        switch (stopReason) {
            case QUANTUM_EXPIRED:
                readyQ.push_back(readyQ.front());
                readyQ.pop_front();
                break;
            case ENTER_IO:
                readyQ.front().pop_front();
                blockedQ.push_back(readyQ.front());
                sortQ(blockedQ);
                readyQ.pop_front();
                break;
            case COMPLETED:
                finished[f].id = cpu_burst->id;
                finished[f].completionTime = total_time;
                finished[f].totalWaitTime = total_time - cpuExecutedTimes[cpu_burst->id] - ioExecutedTimes[cpu_burst->id];
                f++;
                readyQ.pop_front();
                break;
        }
    }

    for (int i = 0; i < processes.size(); i++) {
        log_process_completion(finished[i].id, finished[i].completionTime, finished[i].totalWaitTime);
    }
}

// First Come First Serve scheduling implementation
void fcfs(vector<deque<Burst>> processes) {
    deque<deque<Burst>> blockedQ;
    deque<deque<Burst>> readyQ(processes.begin(), processes.end());

    int ioExecutedTimes[processes.size()] = {0};
    int cpuExecutedTimes[processes.size()] = {0};

    int elapsed_time = 0;
    int total_time = 0;

    Burst* cpu_burst;
    Burst* io_burst;

    ExecutionStopReasonType stopReason;
    completionLog finished[processes.size()];
    int f = 0;

    while (!readyQ.empty() || !blockedQ.empty()) {
        stopReason = ENTER_IO;

        if (readyQ.empty()) {
            io_burst = &(blockedQ.front().front());
            elapsed_time = io_burst->duration;

            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) {
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                } else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }

            for (int i = 0; i < blockedQ.size(); i++) {
                if (blockedQ.front().front().duration == 0) {
                    blockedQ.front().pop_front();
                    readyQ.push_back(blockedQ.front());
                    blockedQ.pop_front();
                }
            }

            total_time += elapsed_time;
            continue;
        }

        cpu_burst = &(readyQ.front().front());
        cpuExecutedTimes[cpu_burst->id] += cpu_burst->duration;
        elapsed_time = cpu_burst->duration;
        cpu_burst->duration = 0;
        stopReason = (readyQ.front().size() != 1) ? ENTER_IO : COMPLETED;

        if (!blockedQ.empty()) {
            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) {
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                } else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }

            for (int i = 0; i < blockedQ.size(); i++) {
                if (blockedQ.front().front().duration == 0) {
                    blockedQ.front().pop_front();
                    readyQ.push_back(blockedQ.front());
                    blockedQ.pop_front();
                }
            }
        }

        total_time += elapsed_time;

        log_cpuburst_execution(cpu_burst->id, cpuExecutedTimes[cpu_burst->id], ioExecutedTimes[cpu_burst->id], total_time, stopReason);

        switch (stopReason) {
            case ENTER_IO:
                readyQ.front().pop_front();
                blockedQ.push_back(readyQ.front());
                sortQ(blockedQ);
                readyQ.pop_front();
                break;
            case COMPLETED:
                finished[f].id = cpu_burst->id;
                finished[f].completionTime = total_time;
                finished[f].totalWaitTime = total_time - cpuExecutedTimes[cpu_burst->id] - ioExecutedTimes[cpu_burst->id];
                f++;
                readyQ.pop_front();
                break;
        }
    }

    for (int i = 0; i < processes.size(); i++) {
        log_process_completion(finished[i].id, finished[i].completionTime, finished[i].totalWaitTime);
    }
}