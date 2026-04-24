#include "manager.h"
#include "shell.h"

#include <fstream>
#include <sstream>


Shell::Shell() : m() {}

Shell::Shell(std::ostream & out) : m(out) {}

bool Shell::valid_args(func & cmd, int & i) {
    // returns True if there are enough arguments inputted (i)
    // for a given command (cmd)
    switch (cmd) {
        case CREATE:
            return i == 1;
        case DESTROY:
            return i == 1;
        case REQUEST:
            return i == 2;
        case RELEASE:
            return i == 2;
        case TIMEOUT:
            return i == 0;
        case SCHEDULER:
            return i == 0;
        case INIT:
            return i == 0;
        default:
            return true;
    }

}

func Shell::get_command(std::string & s) {
    // map strings to the type of command
    if (s == "cr")
        return CREATE;
    if (s == "de")
        return DESTROY;
    if (s == "rq")
        return REQUEST;
    if (s == "rl")
        return RELEASE;
    if (s == "to")
        return TIMEOUT;
    if (s == "in")
        return INIT;
    return INVALID;
}

void Shell::execute(func & cmd, int args[MAX_ARGS]) {
    // call Manager methods given the command and its arguments
    switch (cmd) {
        case CREATE:
            m.create(args[0]);
            break;
        case DESTROY:
            m.destroy(args[0]);
            break;
        case REQUEST:
            m.request(args[0], args[1], m.running_process());
            break;
        case RELEASE:
            m.release(args[0], args[1], m.running_process());
            break;
        case TIMEOUT:
            m.timeout();
            break;
        case SCHEDULER:
            m.scheduler();
            break;
        case INIT:
            m.init();
            break;
        default:
            break;
    }
}

void Shell::execute_line(std::string & l) {
    // executes 1 line (command)
    func cmd;
    int args[MAX_ARGS];
    std::string line;

    // trim whitespaces
    size_t first = l.find_first_not_of(" \r\n\t");
    size_t last = l.find_last_not_of(" \r\n\t");
    if (first == std::string::npos && last == std::string::npos)
        line = "";
    else
        line = l.substr(first, last-first+1);
    
    // skip blank lines
    if (line.length() == 0)
        return;

    // record which function (cmd)
    std::string str;
    std::stringstream ss(line);
    getline(ss, str, ' ');
    cmd = get_command(str);

    // get remaining parameters (args)
    int i=0;
    try {
        for (; i < MAX_ARGS && std::getline(ss, str, ' '); ) {
            if (str != "") {
                args[i] = std::stoi(str);
                ++i;
            }
        }
    } catch (const std::invalid_argument & e) {
        std::cerr << "invalid command" << std::endl;
        return;
    }

    // error if incorrect command format
    if (cmd == INVALID || !valid_args(cmd, i)) {
        std::cerr << "invalid command" << std::endl;
        return;
    }

    // call corresponding Manager methods
    execute(cmd, args);
}
