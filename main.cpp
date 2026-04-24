#include "manager.h"
#include "shell.h"

#include <string>
#include <iostream>
#include <fstream>


void test_manager(Manager & m) {
    // basic testing for Manager

    // m.create(0); // 0
    m.create(1); // 1
    m.create(1); // 2
    m.create(2); // 3 (running)
    m.create(1); // 4

    m.request(0,1,3); // request 1 of resource 0 for process 3
    m.release(0,1,3); // release ^ ^
    // m.request(0,2,3); // request 2 of resource 0 for process 3 (error)
    // m.release(1,1,3); // release 1 of resource 1 for process 3 (error, not held)
    m.request(2,2,3); // request 2 of resource 2 for process 3

    // m.request(2,2,3); // error
    // m.release(2,4,3); // error

    m.request(2,2,2); // request 2 of resource 2 for process 2 (blocked)

    m.destroy(3);
    m.destroy(2);
    m.destroy(1);
    m.destroy(4); // already done in destroy(3)
    m.destroy(0); // can't destroy process 0 (error)


    // for (int i=0; i < 4; ++i) {
    //     std::cout << m.pcb[i].state << ' ' << m.pcb[i].parent << ' ';
    //     for (Node * t = m.pcb[i].children; t != nullptr; t = t->next) {
    //         std::cout << t->index << ' ';
    //     } std::cout << std::endl;
    // }
}

// void print_children(Manager & m) {
//     for (int i=0; i<5; ++i) {
//         std::cout << "\nprocess " << i << ": ";
//         for (Node * c=m.pcb[i].children; c != nullptr; c=c->next) {
//             std::cout << c->index << ' ';
//         }
//     } std::cout << std::endl;
// }

// void print_resources(Manager & m) {
//     for (int i=0; i<5; ++i) {
//         std::cout << "\nprocess " << i << ": ";
//         for (Node * c=m.pcb[i].resources; c != nullptr; c=c->next) {
//             std::cout << c->index << "->" << c->num_req << ' ';
//         }
//     } std::cout << std::endl;
// }

// void print_rl(Manager & m) {
//     for (int i=0; i<PRIORITY_LEVELS; ++i) {
//         std::cout << "\nlevel " << i << ": ";
//         for (Node * c=m.rl[i]; c != nullptr; c=c->next) {
//             std::cout << c->index << ' ';
//         }
//     } std::cout << std::endl;
// }

int run_file(const std::string & in_filename, const std::string & out_filename) {
    // in_filename: name of the file that has shell input
    // out_filename: name of the file to write the shell's output

    // open files
    std::ifstream infile(in_filename);
    std::ofstream outfile(out_filename);

    // error check
    if(!infile.is_open() || !outfile.is_open()) {
        std::cout << "error opening file(s)" << std::endl;
        return 1;
    }

    Shell s(outfile);

    for (std::string line; std::getline(infile, line); ) {
        // std::cout << "INPUT: " << line << std::endl;
        // remove extra escape characters
        size_t last_index = line.find_last_not_of("\r\n\t");
        if (last_index != std::string::npos)
            line.erase(last_index+1);
        else if (line.length() == 1 && last_index == std::string::npos)
            line.erase(0);
    
        s.execute_line(line);
    }

    // close files
    infile.close();
    outfile.close();

    return 0; // success
}

void shell_loop() {
    Shell s;
    // loop over shell
    std::string str;
    while (std::getline(std::cin, str)) { // stop when u type EOF
        s.execute_line(str);

        // testing
        // print_children(s.m);
        // print_rl(s.m);
        // print_resources(s.m);
    }
}

int main(int argc, char ** argv) {
    // All commands n the input file will have a syntactically correct format. 
    // Specifically, only existing opcodes will be given (cr, rq, to, etc.) 
    // followed by the appropriate number of parameters, each separated by a 
    // blank space (as shown in the sample input file).

    if (argc == 1) { // loop over shell, i/o done through command line
        shell_loop();
    } else if (argc == 3) { // given input and output files to run the shell on
        return run_file(argv[1], argv[2]);
    } else { // invalid program input(s)
        std::cout << 
        "Please specify an input file and its output file, or provide no additional arguments to enter commands manually."
        << std::endl;
        return 1;
    }

    return 0;
}
