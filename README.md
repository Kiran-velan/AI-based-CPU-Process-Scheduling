# RL-ProcessScheduler

A process scheduling system powered by reinforcement learning (Q-learning) designed to optimize CPU scheduling decisions based on dynamic system conditions.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)
![Version](https://img.shields.io/badge/version-1.0.0-orange)

## 📋 Overview

RL-ProcessScheduler implements an intelligent CPU scheduler that uses reinforcement learning to make optimal scheduling decisions. Unlike traditional scheduling algorithms (Round Robin, SJF, Priority), this approach continuously learns and adapts to changing system conditions.

### Key Features

- **Adaptive Learning**: Continuously improves scheduling decisions through Q-learning
- **Multi-factor Optimization**: Considers process priority, burst time, waiting time, CPU utilization, and memory usage
- **Anti-starvation Mechanism**: Special handling for processes with long waiting times
- **Exploration vs. Exploitation**: Balances trying new scheduling patterns vs. using proven effective patterns

## 🧠 How It Works

The scheduler uses Q-learning, a reinforcement learning technique, to determine which process to run next:

1. **State Representation**: Each process is in one of several states based on its attributes (remaining burst time, priority, waiting time)
2. **Actions**: Selecting which process to schedule next
3. **Rewards**: Calculated primarily based on how scheduling decisions affect average waiting time
4. **Learning**: The Q-table is updated after each scheduling decision to improve future choices

### State Definitions

- **State 0**: Short remaining time (< 30% of original), any priority - best for SJF approach
- **State 1**: Medium remaining time (< 70%) with high priority
- **State 2**: Any process with very high waiting time (anti-starvation)
- **State 3**: Medium remaining time with medium priority
- **State 4**: Long remaining time, low priority

## 📊 Performance

The scheduler is designed to minimize average waiting time while respecting process priorities. Performance varies based on workload characteristics:

- **Homogeneous workloads**: Achieves near-optimal performance similar to SJF
- **Mixed workloads**: Outperforms fixed algorithms by adapting to workload patterns
- **Priority-sensitive workloads**: Balances throughput with priority requirements

## 🔧 Installation

### Prerequisites

- GCC compiler
- Standard C libraries

### Building from source

```bash
# Clone the repository
git clone https://github.com/Kiran-velan/RL-based-CPU-Process-Scheduling.git
cd RL-ProcessScheduler

# Compile the code
gcc -o rl_scheduler rl_process_scheduler.c -lm

# Run the scheduler
./rl_scheduler
```

## 📝 Usage

### Basic Usage

When you run the program, it will prompt you to enter the number of processes to schedule (up to 10). The system will then:

1. Generate random attributes for each process
2. Display the initial process states
3. Run the RL scheduler until all processes complete
4. Display the final average waiting time

```
Enter the number of processes (max 10): 5
```

### Advanced Configuration

You can modify the following constants in the code to adjust behavior:

- `TIME_QUANTUM`: How long each process runs before scheduling reconsideration
- `LEARNING_RATE`: How quickly the system learns from new information (0-1)
- `DISCOUNT_FACTOR`: How much future rewards are valued compared to immediate rewards (0-1)
- `EPSILON`: Exploration rate for trying new scheduling decisions (0-1)

## 🔍 Code Structure

```
rl_process_scheduler.c
├── Process structure definition
├── Global variables
├── Main function and scheduling loop
├── Process initialization
├── Q-learning implementation
│   ├── Q-table initialization
│   ├── Action selection (select_action)
│   ├── Q-table update function
│   └── Reward calculation
├── State management
│   ├── State determination
│   └── State updates
└── Display and utility functions
```

## 🧪 Examples

### Example 1: Small batch of processes

```
Enter the number of processes (max 10): 3

Initial Processes:
ID	Burst	OrigBurst	Wait	Priority	CPU%	Memory%	State	Completed
0	15	15		0	7		0.42	0.78	4	No
1	4	4		0	3		0.91	0.12	3	No
2	9	9		0	9		0.14	0.29	1	No
Current Average Waiting Time: 0.00

Executing Process ID: 1

Current Process States:
ID	Burst	OrigBurst	Wait	Priority	CPU%	Memory%	State	Completed
0	15	15		2	6		0.33	0.55	4	No
1	2	4		0	2		0.77	0.21	0	No
2	9	9		2	8		0.49	0.61	1	No
Current Average Waiting Time: 1.33

...

All processes completed!
Average Waiting Time: 12.67
```

### Example 2: Process with different priorities

```
Enter the number of processes (max 10): 4

Initial Processes:
ID	Burst	OrigBurst	Wait	Priority	CPU%	Memory%	State	Completed
0	18	18		0	10		0.22	0.45	1	No
1	3	3		0	2		0.81	0.38	3	No
2	12	12		0	5		0.64	0.19	3	No
3	7	7		0	8		0.37	0.63	1	No
Current Average Waiting Time: 0.00

...

All processes completed!
Average Waiting Time: 15.25
```

## 📈 Visualization of Learning Process

The scheduler gradually improves its decision-making through continual updates to the Q-table:

```
Q-Table:
Process	State0	State1	State2	State3	State4
0	0.00	0.00	0.00	0.00	0.00
1	0.00	0.00	0.00	0.00	0.00
2	0.00	0.00	0.00	0.00	0.00
3	0.00	0.00	0.00	0.00	0.00

... after several iterations ...

Q-Table:
Process	State0	State1	State2	State3	State4
0	1.24	0.53	-0.12	0.00	-0.45
1	2.87	0.91	0.28	0.73	-0.22
2	0.95	1.32	1.85	0.00	-0.18
3	1.47	1.69	1.03	0.64	-0.33
```

## 📚 Theory and Implementation Details

### Q-Learning Formula

The core Q-learning update formula used is:

```
Q(s,a) = Q(s,a) + α * [R + γ * max Q(s',a') - Q(s,a)]
```

Where:
- `Q(s,a)` is the value of taking action a in state s
- `α` is the learning rate (how quickly new information overrides old)
- `R` is the reward received after taking action a
- `γ` is the discount factor (value of future rewards)
- `max Q(s',a')` is the maximum Q-value possible in the next state

### Reward Function

The reward function is designed to encourage minimizing average waiting time:

```c
float reward = base_reward + (waiting_time_change * 5.0);
```

Where:
- `base_reward` combines process priority and progress made
- `waiting_time_change` is the difference between previous and current average waiting time

## 🛠 Extending the Project

Possible extensions to this project:

1. **Multiple Resource Consideration**: Extend to consider I/O, memory, and network resources
2. **Deep Q-Network**: Replace Q-table with neural network for more complex state spaces
3. **Dynamic Time Quantum**: Allow time quantum to adjust based on system conditions
4. **Process Dependencies**: Add support for processes with dependencies
5. **Distributed Scheduling**: Extend to distributed systems scheduling

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request


Project Link: [https://github.com/yourusername/RL-ProcessScheduler](https://github.com/Kiran-velan/RL-based-CPU-Process-Scheduling)
