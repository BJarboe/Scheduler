# Process Scheduler
[See example](#example)
## Overview
This project simulates process scheduling using CPU and IO bursts. It supports two scheduling strategies:
- **First Come First Serve (FCFS)**
- **Round Robin (RR)**

The program reads process burst data from a file and executes the scheduling strategy specified via command-line arguments.

## Features
- **FCFS**: Processes are executed in the order they arrive.
- **RR**: Processes are executed in time slices (quantum) with context switching.
- Logs CPU and IO burst execution details.
- Calculates turnaround and wait times for each process.

## Usage
```bash
./schedule [Burst file] -s [Scheduling strategy] -q [Time Quantum]
```
## Example
**Round Robin with 3ms quantum**
```bash
./schedule bursts.txt -s rr -q 3
```
**Burst file example**
```
4 4 2 
1 7 3 
3 2 4 
```
**Round Robin Output**
```
P0: executed cpu bursts = 4, executed io bursts = 0, time elapsed = 4, enter io
P1: executed cpu bursts = 1, executed io bursts = 0, time elapsed = 5, enter io
P2: executed cpu bursts = 3, executed io bursts = 0, time elapsed = 8, enter io
P0: executed cpu bursts = 6, executed io bursts = 4, time elapsed = 10, completed
P2: executed cpu bursts = 7, executed io bursts = 2, time elapsed = 14, completed
P1: executed cpu bursts = 4, executed io bursts = 7, time elapsed = 17, completed
P0: turnaround time = 10, wait time = 0
P2: turnaround time = 14, wait time = 5
P1: turnaround time = 17, wait time = 6
```
