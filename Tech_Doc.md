# Technical Documentation: RL-ProcessScheduler

## Introduction to Reinforcement Learning in Process Scheduling

This document provides a detailed explanation of the reinforcement learning approach used in this process scheduler, including the theoretical foundations, implementation choices, and optimization techniques.

## 1. Theoretical Framework

### 1.1 Reinforcement Learning Basics

Reinforcement Learning (RL) is a machine learning paradigm where an agent learns to make decisions by receiving rewards or penalties from an environment. In the context of our process scheduler:

- **Agent**: The scheduling algorithm
- **Environment**: The CPU and collection of processes
- **State**: The current condition of processes (burst time, waiting time, priority, etc.)
- **Action**: The selection of a process to run next
- **Reward**: Feedback based on scheduling performance (primarily average waiting time)

### 1.2 Q-Learning Algorithm

Q-learning is a model-free RL algorithm that learns the value of an action in a particular state. The core of Q-learning is the Q-table, which stores these state-action values.

#### Q-Value Update Formula:

![Q-learning Formula](https://latex.codecogs.com/png.latex?Q%28s%2Ca%29%20%3D%20Q%28s%2Ca%29%20&plus;%20%5Calpha%20%5Ccdot%20%5BR%20&plus;%20%5Cgamma%20%5Ccdot%20%5Cmax_%7Ba%27%7D%20Q%28s%27%2Ca%27%29%20-%20Q%28s%2Ca%29%5D)

Where:
- `Q(s,a)` = Current estimate of state-action value
- `α` = Learning rate (0.2 in our implementation)
- `R` = Reward received after taking action a in state s
- `γ` = Discount factor for future rewards (0.9 in our implementation)
- `maxₐ' Q(s',a')` = Maximum Q-value possible in the next state

### 1.3 Epsilon-Greedy Strategy

To balance exploration (trying new scheduling patterns) with exploitation (using known good patterns), we use an epsilon-greedy strategy:

- With probability `ε` (0.1 in our code), choose a random process
- With probability `1-ε`, choose the process with the highest Q-value for its current state

## 2. State Space Design

### 2.1 Process States

Each process exists in one of five discrete states based on its attributes:

| State | Description | Condition |
|-------|-------------|-----------|
| 0 | Short remaining time | `remaining_ratio < 0.3` |
| 1 | Medium remaining with high priority | `remaining_ratio < 0.7 && norm_priority > 0.7` |
| 2 | High waiting time (anti-starvation) | `norm_waiting > 0.8` |
| 3 | Medium remaining with medium priority | `remaining_ratio < 0.7 && norm_priority > 0.3` |
| 4 | Long remaining, low priority | All other cases |

### 2.2 State Transition

Process states change based on:
1. Execution (reducing burst time)
2. Waiting (increasing waiting time)
3. Dynamic changes to system priority

## 3. Reward Function Design

### 3.1 Component Analysis

The reward function is critical for proper learning and has been designed to optimize average waiting time:

```c
float waiting_time_change = prev_avg_waiting_time - current_avg_waiting;
    
float base_reward = (0.5 * (float)p.system_priority / 10.0) +
                    (0.3 * (1.0 - (float)p.burst_time / p.original_burst_time));
    
float reward = base_reward + (waiting_time_change * 5.0);
```

Components:
1. **waiting_time_change**: The difference between previous and current average waiting time
   - Positive when average waiting time decreases (good)
   - Negative when average waiting time increases (bad)
   
2. **base_reward**: Combines:
   - Process priority (normalized to 0-1)
   - Process completion progress (higher when more of the process is completed)
   
3. **Weighting factor (5.0)**: Emphasizes the importance of waiting time change

### 3.2 Reward Optimization

The emphasis on waiting time change directly incentivizes scheduling decisions that reduce the overall average waiting time. This is weighted significantly higher than other factors.

## 4. Implementation Details

### 4.1 Data Structures

#### Process Structure
```c
typedef struct {
    int id;
    int burst_time;
    int original_burst_time;
    int waiting_time;
    int system_priority;
    float cpu_utilization;
    float memory_usage;
    bool completed;
    int current_state;
} Process;
```

#### Q-Table
The Q-table is implemented as a 2D array:
```c
float Q_table[MAX_PROCESSES][MAX_STATES];
```
- Each row represents a process
- Each column represents a state
- The value at Q_table[i][j] represents the expected utility of scheduling process i when it's in state j

### 4.2 Key Algorithms

#### Action Selection
```c
int select_action() {
    // Implement epsilon-greedy strategy
    if ((float)rand() / RAND_MAX < EPSILON) {
        // Exploration: choose a random non-completed process
        ...
    } else {
        // Exploitation: choose process with highest Q-value for its current state
        ...
    }
}
```

#### Q-Table Update
```c
void update_q_table(int process_id, int old_state, int new_state) {
    // Calculate reward
    float reward = calculate_reward(processes[process_id], current_avg_waiting);

    // Find max Q-value for the new state
    float max_next_q = ...

    // Update Q-value using Q-learning formula  
    Q_table[process_id][old_state] = Q_table[process_id][old_state] + 
                                    LEARNING_RATE * (reward + DISCOUNT_FACTOR * max_next_q - 
                                    Q_table[process_id][old_state]);
}
```

### 4.3 Time Complexity Analysis

- **Initialization**: O(n), where n is the number of processes
- **Action Selection**: O(n), must check each non-completed process
- **Q-table Update**: O(s), where s is the number of states (constant in our implementation)
- **Overall Algorithm**: O(n²), as each process may require up to n scheduling decisions

## 5. Performance Optimization

### 5.1 Starvation Prevention

To prevent process starvation, we've implemented:
1. A dedicated state (State 2) for processes with high waiting times
2. A reward component based on waiting time reductions

### 5.2 Shortest Job First Bias

For efficiency, the state design favors processes with short remaining times (State 0), implementing a form of SJF (Shortest Job First) when appropriate.

### 5.3 Priority Consideration

While minimizing waiting time is the primary goal, the system respects process priorities through:
1. Dedicated states for high-priority processes
2. A base reward component for priority

## 6. Comparison with Traditional Algorithms

| Algorithm | Advantages | Disadvantages | When RL is Better |
|-----------|------------|---------------|-------------------|
| **FCFS (First-Come-First-Served)** | Simple, no starvation | Poor average waiting time, no priority | Almost always |
| **SJF (Shortest Job First)** | Optimal average waiting time | Potential starvation, requires known burst times | With variable/unknown burst times |
| **Priority Scheduling** | Respects importance | Starvation of low-priority processes | When balancing multiple factors |
| **Round Robin** | Fair time sharing | Poor for varied burst times | For heterogeneous workloads |

Our RL approach combines the strengths of multiple scheduling algorithms and adapts to the specific workload characteristics.

## 7. Parameter Tuning

### 7.1 Critical Parameters

| Parameter | Range | Effect | Recommended Value |
|-----------|-------|--------|------------------|
| TIME_QUANTUM | 1-10 | How long each process runs before rescheduling | 2 (balance responsiveness and overhead) |
| LEARNING_RATE | 0.0-1.0 | How quickly the system adapts to new information | 0.2 (stable learning) |
| DISCOUNT_FACTOR | 0.0-1.0 | Value of future rewards vs. immediate rewards | 0.9 (forward-looking) |
| EPSILON | 0.0-1.0 | Exploration rate | 0.1 (10% exploration) |

### 7.2 Tuning Methodology

To optimize these parameters for a specific workload:

1. Start with recommended values
2. Perform a grid search over parameter ranges
3. Evaluate using average waiting time and fairness metrics
4. Select the parameter set with best performance

## 8. Known Limitations

1. **Learning Delay**: The system needs time to learn optimal policies, which may result in suboptimal scheduling initially
2. **Parameter Sensitivity**: Performance depends on proper tuning of learning parameters
3. **State Space Granularity**: Limited number of states may not capture all relevant process characteristics
4. **Random Process Generation**: In real systems, processes have more complex patterns and dependencies

## 9. Future Enhancements

### 9.1 Technical Improvements

1. **Deep Q-Network (DQN)**: Replace Q-table with neural network for more complex state representation
2. **Prioritized Experience Replay**: Store and reuse significant scheduling decisions to improve learning
3. **Multi-resource Scheduling**: Extend to consider I/O, memory, and network resources
4. **Process Type Awareness**: Different scheduling strategies for interactive vs. batch processes

### 9.2 Implementation Enhancements

1. **Configuration File**: Allow parameter adjustment without recompilation
2. **Statistics Logging**: Record detailed performance metrics for analysis
3. **Visualization Tools**: Real-time visualization of scheduling decisions and learning progress
4. **Workload Profiles**: Predefined process sets to benchmark against standard scheduling algorithms

## 10. Troubleshooting Guide

### 10.1 Common Issues

1. **High Average Waiting Time**
   - Check reward function weighting
   - Increase exploration rate temporarily
   - Verify state definitions match workload characteristics

2. **Process Starvation**
   - Adjust State 2 threshold (norm_waiting)
   - Increase weight of waiting time in reward function

3. **Unstable Learning**
   - Reduce learning rate
   - Verify state transitions are consistent

### 10.2 Debugging Techniques

1. Track Q-value evolution for specific processes
2. Log state transitions and rewards
3. Compare with baseline algorithms (FCFS, SJF) for same workload
