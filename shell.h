#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <iostream>

// max number of arguments over all commands
#define MAX_ARGS 2

enum func {CREATE, DESTROY, REQUEST, RELEASE, TIMEOUT, SCHEDULER, INIT, INVALID};


class Shell {
// public:
    Manager m;

    bool valid_args(func & cmd, int & i);
    func get_command(std::string & s);
    void execute(func & cmd, int args[MAX_ARGS]);

public:
    Shell();
    Shell(std::ostream & out);
    void execute_line(std::string & l);
};

#endif // SHELL_H
