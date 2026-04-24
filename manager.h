#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>

// process states
#define READY   1
#define BLOCKED 0
#define FREE   -1

// resource states
// #define FREE      1
// #define ALLOCATED 0

// array lengths
#define N 16  // PCB array
#define M  4  // RCB array

// multiunit resources
#define RCB0_AMT 1
#define RCB1_AMT 1
#define RCB2_AMT 2
#define RCB3_AMT 3

// number of priority levels for RL
#define PRIORITY_LEVELS 3


struct Node { // for the singly linked lists
    int index;   // process number (index into pcb)
    int num_req; // # of resources required for proecess
    Node * next;

    Node(const int & index, Node * next);
    Node(const int & index, const int & num_req, Node * next);

    static void delete_sll(Node * n);
    static Node * delete_node(Node * n, const int & val);
    static Node * find(Node * n, const int & val);
    static Node * last_node(Node * n);
};


struct PCB { // process control block
    int priority; // 0,1,2
    int state;    // either READY or BLOCKED
    int parent;   // index of parent process (-1 for process 0)
    Node * children;
    Node * resources;

    PCB();
};


struct RCB { // resource control block
    int inventory;     // # of initial units
    int state;         // how many units are still available
    Node * waitlist;   // process index + amount requested

    RCB();
};


class Manager {
// public:
    int n; // keep track of # of processes destroyed
    bool first_init; // flag to know if the next "in" command's starts 
                     // on a new line (so it matches the expected output)
    std::ostream * out;
    // data structures
    PCB  pcb[N]; // process control block array
    RCB  rcb[M]; // resource control block array
    Node * rl[PRIORITY_LEVELS]; // ready list

    void clear_data();
    int find_empty_PCB();
    void create_0();
    void recursive_destroy(int j);
    void release_unblock_loop(int r);
    bool in_subtree(const int & i, const int & child);
    void error();

public:
    Manager(std::ostream & out=std::cout);
    ~Manager();
    void set_output(std::ostream & out);
    int running_process();

    void create(int p);
    void destroy(int j);
    void request(int r, int k, int i);
    void release(int r, int k, int i);
    void timeout();
    void scheduler();
    void init();
};

#endif // MANAGER_H
