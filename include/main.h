#ifndef MAIN_H
#define MAIN_H

/* Number of interpreters that we support.  One of these will be invoked
 * when we receive a client connection */
#define NUM_INTERPRETERS 3

/* Defines the types of interpreters supported */
enum interpreter
{
    PERL = 0,
    PYTHON = 1,
    SH = 2
};

struct options
{
    /* General */
    char* user;
    char* umask;
    char* logfile_path;
    char* errfile_path;
    char* config_path;
    char* script_path;

    /* Networking */
    char* port;
    int max_instances;
    int backlog;
    int ipver;

    /* Command script info */
    enum interpreter interpreter;
    char* perl_path;
    char* python_path;
    char* sh_path;
};

/* Cleanly exits the server, returning STATUS as the prgram's exit status */
void exit_program (int status);

#endif //MAIN_H

