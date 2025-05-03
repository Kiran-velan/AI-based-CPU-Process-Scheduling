#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_PROCESSES 10
#define MAX_STATES 5
#define TIME_QUANTUM 2
#define LEARNING_RATE 0.2
#define DISCOUNT_FACTOR 0.9
#define EPSILON 0.1 // Exploration factor

// Process structure
typedef struct {
    int id;
    int burst_time;
    int original_burst_time; // To track original burst time
    int waiting_time;
    int system_priority;
    float cpu_utilization;
    float memory_usage;
    bool completed;
    int current_state; // Added to track the current state of each process
} Process;

// Global variables
Process processes[MAX_PROCESSES];
float Q_table[MAX_PROCESSES][MAX_STATES];
int num_processes;
float prev_avg_waiting_time = 0; // Track previous average waiting time

// Function prototypes
void initialize_processes();
void initialize_q_table();
int select_action();
void update_q_table(int process_id, int old_state, int new_state);
float calculate_reward(Process p, float current_avg_waiting);
int determine_state(Process p);
void update_state(int process_id);
void remove_completed_process(int process_id);
void display_processes();
void display_q_table();
float calculate_avg_waiting_time();

int main() {
    srand(time(NULL));
    
    initialize_processes();
    initialize_q_table();

    printf("Initial Processes:\n");
    display_processes();

    // Display initial Q-table
    display_q_table();

    while (1) {
        // Check if all processes are completed
        bool all_completed = true;
        for (int i = 0; i < num_processes; i++) {
            if (!processes[i].completed) {
                all_completed = false;
                break;
            }
        }
        if (all_completed) break;

        // Select the process to schedule based on Q-value
        int selected_process = select_action();
        Process *p = &processes[selected_process];
        
        // Store the old state before execution
        int old_state = p->current_state;

        // Execute the process for the time quantum
        printf("\nExecuting Process ID: %d\n", p->id);
        p->burst_time -= TIME_QUANTUM;
        if (p->burst_time <= 0) {
            p->burst_time = 0;
            p->completed = true;
        }

        // Update waiting times of other processes
        for (int i = 0; i < num_processes; i++) {
            if (i != selected_process && !processes[i].completed) {
                processes[i].waiting_time += TIME_QUANTUM;
            }
        }

        // Update dynamic state variables
        update_state(selected_process);
        int new_state = determine_state(*p);
        p->current_state = new_state;

        // Calculate current average waiting time
        float current_avg_waiting = calculate_avg_waiting_time();
        
        // Update Q-table with the new state information
        update_q_table(selected_process, old_state, new_state);

        // Update previous average waiting time
        prev_avg_waiting_time = current_avg_waiting;

        // Remove completed process from Q-table
        if (p->completed) {
            remove_completed_process(selected_process);
        }

        // Display updated process table and Q-table
        display_processes();
        display_q_table();
    }

    printf("\nAll processes completed!\n");

    float avg_waiting_time = calculate_avg_waiting_time();
    printf("Average Waiting Time: %.2f\n", avg_waiting_time);

    return 0;
}

void initialize_processes() {
    printf("Enter the number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", &num_processes);
    for (int i = 0; i < num_processes; i++) {
        processes[i].id = i;
        processes[i].burst_time = rand() % 20 + 1; // Random burst time between 1 and 20
        processes[i].original_burst_time = processes[i].burst_time; // Store original burst time
        processes[i].waiting_time = 0;
        processes[i].system_priority = rand() % 10 + 1; // Random priority between 1 and 10
        processes[i].cpu_utilization = (float)(rand() % 100) / 100.0; // Random CPU utilization (0-1)
        processes[i].memory_usage = (float)(rand() % 100) / 100.0; // Random memory usage (0-1)
        processes[i].completed = false;
        processes[i].current_state = determine_state(processes[i]); // Initialize state
    }
}

void initialize_q_table() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        for (int j = 0; j < MAX_STATES; j++) {
            Q_table[i][j] = 0.0; // Initialize to zero
        }
    }
}

int select_action() {
    // Implement epsilon-greedy strategy
    if ((float)rand() / RAND_MAX < EPSILON) {
        // Exploration: choose a random non-completed process
        int count = 0;
        int non_completed[MAX_PROCESSES];
        for (int i = 0; i < num_processes; i++) {
            if (!processes[i].completed) {
                non_completed[count++] = i;
            }
        }
        return non_completed[rand() % count];
    } else {
        // Exploitation: choose process with highest Q-value for its current state
        float max_q_value = -1e9;
        int best_process = -1;

        for (int i = 0; i < num_processes; i++) {
            if (!processes[i].completed) {
                int current_state = processes[i].current_state;
                if (Q_table[i][current_state] > max_q_value) {
                    max_q_value = Q_table[i][current_state];
                    best_process = i;
                }
            }
        }
        
        // Fallback if no best process found (shouldn't happen)
        if (best_process == -1) {
            for (int i = 0; i < num_processes; i++) {
                if (!processes[i].completed) {
                    best_process = i;
                    break;
                }
            }
        }
        
        return best_process;
    }
}

void update_q_table(int process_id, int old_state, int new_state) {
    if (processes[process_id].completed) return; // Skip completed processes

    // Calculate current average waiting time
    float current_avg_waiting = calculate_avg_waiting_time();
    
    // Calculate reward for the action - pass current average waiting time
    float reward = calculate_reward(processes[process_id], current_avg_waiting);

    // Find max Q-value for the new state
    float max_next_q = 0;
    for (int j = 0; j < MAX_STATES; j++) {
        if (Q_table[process_id][j] > max_next_q) {
            max_next_q = Q_table[process_id][j];
        }
    }

    // Update Q-value for the old state using the Q-learning formula
    Q_table[process_id][old_state] = Q_table[process_id][old_state] + 
                                    LEARNING_RATE * (reward + DISCOUNT_FACTOR * max_next_q - 
                                    Q_table[process_id][old_state]);
}

float calculate_reward(Process p, float current_avg_waiting) {
    // The key goal: minimize average waiting time
    float waiting_time_change = prev_avg_waiting_time - current_avg_waiting;
    
    // Base reward from process attributes
    float base_reward = (0.5 * (float)p.system_priority / 10.0) +  // Higher priority (normalized)
                        (0.3 * (1.0 - (float)p.burst_time / p.original_burst_time)); // Progress made
    
    // The main reward component is the change in average waiting time
    // If average waiting time decreased, give positive reward
    // If it increased, give negative reward
    float reward = base_reward + (waiting_time_change * 5.0);
    
    return reward;
}

// Determine the state based on process attributes
int determine_state(Process p) {
    // More effective state mapping focused on scheduling factors
    
    // Remaining burst time ratio (0-1)
    float remaining_ratio = p.original_burst_time > 0 ? 
                          (float)p.burst_time / p.original_burst_time : 0;
    
    // Normalized waiting time (higher is worse)
    float norm_waiting = (float)p.waiting_time / 20.0; // Normalize to approx 0-1 range
    if (norm_waiting > 1.0) norm_waiting = 1.0;
    
    // Normalized priority (higher is better)
    float norm_priority = (float)p.system_priority / 10.0;
    
    // State mapping:
    // State 0: Short remaining time, any priority (best for SJF)
    if (remaining_ratio < 0.3) return 0;
    
    // State 1: Medium remaining with high priority
    if (remaining_ratio < 0.7 && norm_priority > 0.7) return 1;
    
    // State 2: Any remaining with very high wait time (avoid starvation)
    if (norm_waiting > 0.8) return 2;
    
    // State 3: Medium remaining with medium priority
    if (remaining_ratio < 0.7 && norm_priority > 0.3) return 3;
    
    // State 4: Everything else (long remaining, low priority)
    return 4;
}

void update_state(int process_id) {
    // Only update dynamic attributes
    // Keep burst time and waiting time as they are - they're updated in main()
    processes[process_id].system_priority = rand() % 10 + 1;
    processes[process_id].cpu_utilization = (float)(rand() % 100) / 100.0;
    processes[process_id].memory_usage = (float)(rand() % 100) / 100.0;
}

void remove_completed_process(int process_id) {
    for (int i = 0; i < MAX_STATES; i++) {
        Q_table[process_id][i] = -999.0; // Use a very negative value to ensure it's not selected
    }
    printf("Process ID %d completed and removed from Q-table.\n", processes[process_id].id);
}

void display_processes() {
    printf("\nCurrent Process States:\n");
    printf("ID\tBurst\tOrigBurst\tWait\tPriority\tCPU%%\tMemory%%\tState\tCompleted\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%d\t%d\t%d\t\t%d\t%d\t\t%.2f\t%.2f\t%d\t%s\n",
               processes[i].id, processes[i].burst_time, processes[i].original_burst_time,
               processes[i].waiting_time, processes[i].system_priority, 
               processes[i].cpu_utilization, processes[i].memory_usage, 
               processes[i].current_state, processes[i].completed ? "Yes" : "No");
    }
    
    printf("Current Average Waiting Time: %.2f\n", calculate_avg_waiting_time());
}

void display_q_table() {
    printf("\nQ-Table:\n");
    printf("Process\tState0\tState1\tState2\tState3\tState4\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%d\t", i);
        for (int j = 0; j < MAX_STATES; j++) {
            printf("%.2f\t", Q_table[i][j]);
        }
        printf("\n");
    }
}

float calculate_avg_waiting_time() {
    int total_waiting_time = 0;
    int active_count = 0;
    
    for (int i = 0; i < num_processes; i++) {
        total_waiting_time += processes[i].waiting_time;
        active_count++;
    }
    
    return active_count > 0 ? (float)total_waiting_time / active_count : 0;
}
