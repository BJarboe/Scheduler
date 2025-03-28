# Process Scheduler

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