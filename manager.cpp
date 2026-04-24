#include "manager.h"


// helper stuff
Node::Node(const int & index, Node * next) :
    index(index), num_req(0), next(next) {}

Node::Node(const int & index, const int & num_req, Node * next) :
    index(index), num_req(num_req), next(next) {}

PCB::PCB() :
    priority(0), state(FREE), parent(FREE), children(nullptr), resources(nullptr) {}

RCB::RCB() :
    inventory(0), state(0), waitlist(nullptr) {}

void Node::delete_sll(Node * n) {
    // delete the given node and all the ones after it
    Node * tmp;
    while (n != nullptr) {
        tmp = n->next;
        delete n;
        n = tmp;
    }
}

Node * Node::delete_node(Node * n, const int & val) {
    // delete the node with the given value, returns the head of the sll
    Node * prev = nullptr;
    Node * curr = n;
    Node * next;
    for (; curr != nullptr; curr=curr->next) {
        if (curr->index == val) {
            next = curr->next; // keep track of the sll after curr
            delete curr;
            if (prev == nullptr) { return next; } // n (curr) was the head
            prev->next = next;
            break;
        }
        prev = curr;
    } return n;
}

Node * Node::find(Node * n, const int & val) {
    // return the Node where val == index
    for (; n != nullptr; n=n->next) {
        if (n->index == val)
            return n;
    } return nullptr;
}

Node * Node::last_node(Node * n) {
    // return a pointer to the last node in a singly linked list (or nullptr, if n=nullptr)
    if (n == nullptr) { return nullptr; }
    for (; n->next != nullptr; n = n->next);
    return n;
}

// -------- manager ------------
// each function needs error checking:
//     creating more than n processes
//     destroying a process that is not a child of the current process
//     requesting a nonexistent resource
//     requesting a resource the process is already holding
//     releasing a resource the process is not holding
//     process 0 should be prevented from requesting any resource to avoid the possibility of a deadlock where no process is on the RL

Manager::Manager(std::ostream & out) : n{}, first_init{true}, out{&out}, pcb{}, rcb{}, rl{} {
    // instantiate manager:
    // - process descriptor array PCB[16] (done statically)
    // - resource descriptor array RCB[4] (done statically) with multiunit resources
    //   - RCB[0] and RCB[1] with 1 unit each
    //   - RCB[2] has 2 units
    //   - RCB[3] has 3 units
    // - a 3-level RL (done statically)

    // set RCB values
    rcb[0].inventory = RCB0_AMT; // 1
    rcb[1].inventory = RCB1_AMT; // 1
    rcb[2].inventory = RCB2_AMT; // 2
    rcb[3].inventory = RCB3_AMT; // 3

    for (int i=0; i<M; ++i)
        rcb[i].state = rcb[i].inventory;

    // When the system is initialized, process 0 is created automatically 
    // and it becomes the first running process. All other processes 
    // are created and destroyed dynamically using the following operations.
    // create_0();
}

Manager::~Manager() {
    clear_data();
}

void Manager::clear_data() {
    // delete dynamic memory used by PCB,RCB,RL

    // erase PCB
    for (int i=0; i < N; ++i) {
        pcb[i].state = FREE;
        Node::delete_sll(pcb[i].children);  pcb[i].children = nullptr;
        Node::delete_sll(pcb[i].resources); pcb[i].resources = nullptr;
    }

    // erase RCB
    for (int i=0; i < M; ++i) {
        rcb[i].state = rcb[i].inventory;
        Node::delete_sll(rcb[i].waitlist);  rcb[i].waitlist = nullptr;
    }

    // erase RL
    for (int i=0; i < PRIORITY_LEVELS; ++i) {
        Node::delete_sll(rl[i]);
        rl[i] = nullptr;
    }
}

int Manager::find_empty_PCB() {
    // find the leftmost empty PCB array index
    // returns -1 if there's nothing empty
    for (int i=0; i < N; ++i) {
        if (pcb[i].state == FREE)
            return i;
    } return -1;
}

void Manager::create_0() {
    // create the 0th process

    // allocate new pcb[j]
    pcb[0].priority = 0;
    pcb[0].state = READY;
    pcb[0].parent = -1;
    pcb[0].children = nullptr;
    pcb[0].resources = nullptr;

    // insert process 0 into ready list (assuming RL is completely empty)
    rl[0] = new Node{0, nullptr};

    // display message
    // *out << "process " << 0 << " created" << std::endl;

    // call scheduler
    scheduler();
}

void Manager::recursive_destroy(int j) {
    // recursive helper function for destroy, destroys process j and its children

    // increment # of processes destroyed
    ++n;

    // recursively destroy children of i
    for (Node * child = pcb[j].children; child != nullptr; child = pcb[j].children) {
        recursive_destroy(child->index);
    }
    // remove j from parent's list
    Node * & parents_children = pcb[pcb[j].parent].children;
    parents_children = Node::delete_node(parents_children, j);

    // remove j from RL
    if (pcb[j].state == READY) {
        int p = pcb[j].priority;
        rl[p] = Node::delete_node(rl[p], j);
    } else {
    // remove j from waitlist
        for (int m=0; m<M; ++m) { // go through each different resource to find and remove process j
            rcb[m].waitlist = Node::delete_node(rcb[m].waitlist, j);
        }
    }

    // release resources of j
    for (Node * res=pcb[j].resources; res != nullptr; res=pcb[j].resources) {
        int r = res->index;
        int n = res->num_req;

        // remove resource r from resource list of process j
        pcb[j].resources = res->next;
        delete res;
        
        // update number of resources available in rcb array
        rcb[r].state += n;

        // unblock other processes waiting on resource r
        release_unblock_loop(r);
    }

    // free PCB of j
    pcb[j].state = FREE; // invalid state
}

void Manager::release_unblock_loop(int r) {
    // loop to unblock processes (on resource r)
    Node * next = nullptr;
    for (Node * ptr = rcb[r].waitlist; ptr != nullptr && rcb[r].state > 0; ptr=next) {
        int j = ptr->index;   // process j
        int k = ptr->num_req; // k amount of resource r
        next = ptr->next;     // next Node on the waitlist

        if (rcb[r].state >= k) { // can unblock process j
            rcb[r].state -= k;

            // insert resource r into resources of process j
            pcb[j].resources = new Node{r, k, pcb[j].resources};

            // unblock process j
            pcb[j].state = READY;

            // remove process j from waitlist (of resource r)
            rcb[r].waitlist = Node::delete_node(rcb[r].waitlist, j);

            // insert process j into RL
            Node * node = Node::last_node(rl[pcb[j].priority]);
            if (node == nullptr)
                rl[pcb[j].priority] = new Node{j,nullptr};
            else
                node->next = new Node{j,nullptr};
        } // else { break; }
    }
}

bool Manager::in_subtree(const int & i, const int & j) {
    // true if j is a child of i, or if i==j
    if (i == j) { return true; }
    for (Node * child=pcb[i].children; child != nullptr; child=child->next) {
        if (in_subtree(child->index, j)) { return true; }
    } return false;
}

void Manager::error() {
    // prints error message
    // *out << "error" << std::endl;
    *out << -1;
}

void Manager::set_output(std::ostream & out) {
    // change where the output of the manager is written to
    this->out = & out;
}

int Manager::running_process() {
    // return the index of the currently running process (head of the ready list)
    for (int i=PRIORITY_LEVELS-1; i >= 0; --i) {
        if (rl[i] != nullptr)
            return rl[i]->index;
    } return -1; // no running processes
}

void Manager::create(int p) {
    // currently running process, i, can create a new child process, j
    // p is the priority level (0 is reserved for the init process)
    int i = running_process();
    int j = find_empty_PCB();

    // error checking
    if (j == -1) // creating more than n processes
        { error(); return; }
    if (p <= 0 || p >= PRIORITY_LEVELS) // invalid priority level
        { error(); return; }

    // allocate new pcb[j]
    pcb[j].priority = p;
    pcb[j].state = READY;
    pcb[j].parent = i;
    pcb[j].children = nullptr;
    pcb[j].resources = nullptr;

    // insert j into children of i (assuming j is not the 0th process)
    pcb[i].children = new Node{j, pcb[i].children};

    // insert j into ready list
    Node * rl_end = Node::last_node(rl[p]);
    if (rl_end == nullptr)
        rl[p] = new Node{j, nullptr};
    else
        rl_end->next = new Node{j, nullptr};

    // display message
    // *out << "process " << j << " created" << std::endl;

    // call scheduler
    scheduler();
}

void Manager::destroy(int j) {
    // destroy process j and its children

    // counting n destroyed processes
    n = 0;

    // error checking
    int curr = running_process();
    if (!in_subtree(curr, j)) // can only destroy the currently running process or its children
        { error(); return; }
    // if (pcb[curr].state == FREE) // can't destroy a non existent process
    //     { error(); return; }
    if (j == 0) // can't destroy the 0th process
        { error(); return; }

    // destroy j
    recursive_destroy(j);

    // display message
    // *out << n << " processes destroyed" << std::endl;

    // call scheduler()
    scheduler();
}

void Manager::request(int r, int k, int i) {
    // request k units of resource r for process i
    
    // error checks
    if (i == 0) // process 0 should be prevented from requesting any resource to prevent deadlock (no process on RL)
        { error(); return; }
    if (r < 0 || r > M) // nonexistent resource
        { error(); return; }
    if (k < 0) // requesting negative resources
        { error(); return; }
    Node * res = Node::find(pcb[i].resources, r);
    if ((res != nullptr) && (k + res->num_req > rcb[r].inventory)) // can't request more than inventory
        { error(); return; }
    else if (k > rcb[r].inventory)
        { error(); return; }

    // state of r is free
    if (rcb[r].state >= k) {
        rcb[r].state -= k; // allocate state of r
        
        // insert r into list of resources of process i
        if (res == nullptr)
            pcb[i].resources = new Node{r, k, pcb[i].resources};
        else
            res->num_req += k;
        
        // display message
        // *out << "resource " << r << " allocated" << std::endl;
    } else { // block otherwise (no partial allocation of resources)
        pcb[i].state = BLOCKED;
        
        // move i from RL to waitlist of r
        int p = pcb[i].priority;
        rl[p] = Node::delete_node(rl[p], i);
        if (rcb[r].waitlist == nullptr) {
            rcb[r].waitlist = new Node{i, k, rcb[r].waitlist};
        } else { // insert at end of waitlist
            Node * last = Node::last_node(rcb[r].waitlist);
            last->next = new Node{i, k, nullptr};
        }
        
        // display message
        // *out << "process " << i << " blocked" << std::endl;
        
        // call scheduler
        // scheduler();
    }

    // call scheduler
    scheduler();
}

void Manager::release(int r, int k, int i) {
    // release k units of resource r for process i
    // call_scheduler: false when calling release() through recursive_destroy()
    //                 so 

    // error checks
    if (r < 0 || r > M) // nonexistent resource
        { error(); return; }
    if (k < 0) // releasing negative resources
        { error(); return; }
    Node * res = Node::find(pcb[i].resources, r);
    if (res == nullptr)   // release not-held resource
        { error(); return; }
    if (k > res->num_req) // release more units than what's being held
        { error(); return; }

    // remove k of resource r from resource list of process i
    if (k == res->num_req) { // releasing all of resource r held by process i
        pcb[i].resources = Node::delete_node(pcb[i].resources, r);
    } else { res->num_req -= k; } // reduce the amount of resource r held by process i
    
    // update number of resources available in rcb array
    rcb[r].state += k;

    // unblock processes waiting on resource r
    release_unblock_loop(r);
    
    // display message
    // *out << "resource " << r << " released" << std::endl;

    // call scheduler
    scheduler();
}

void Manager::timeout() {
    // move process i from head of RL to end of RL

    // get info
    int i = running_process();
    int p = pcb[i].priority;
    Node * end = Node::last_node(rl[p]);

    // move
    end->next = rl[p];
    rl[p] = rl[p]->next;
    end->next->next = nullptr;

    // call scheduler
    scheduler();
}

void Manager::scheduler() {
    // prints the index of the currently running process
    
    // find process i currently at the head of the RL
    int i = running_process();

    // display message
    // *out << "process " << i << " running" << std::endl;
    *out << i << ' ';
}

void Manager::init() {
    // all PCB entries are initialized to free except PCB[0]
    // PCB[0] is initialized to be a running process with no parent, no children, and no resources
    // all RCB entries are initialized to free
    // RL contains process 0

    clear_data();

    // 'Start a new line for each "in" command'
    if (first_init)
        first_init = false;
    else
        *out << std::endl;

    // create a single running process at PCB[0] with priority 0
    // enter the process into the RL at the lowest priority level 0
    create_0();
}
