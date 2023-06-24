#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory>

using namespace std;

/*
  Function Declarations for built-in shell commands:
 */
int change_directory(vector<string>& args);
int display_help(vector<string>& args);
int exit_shell(vector<string>& args);
int display_history(vector<string>& args);

/*
  List of built-in commands, followed by their corresponding functions.
 */
vector<string> built_in_commands = {
    "cd",
    "help",
    "exit",
    "history"
};

int (*built_in_functions[])(vector<string>&) = {
    &change_directory,
    &display_help,
    &exit_shell,
    &display_history
};

struct Node
{
    string command;
    shared_ptr<Node> next;
};

shared_ptr<Node> head = nullptr;
shared_ptr<Node> cur = nullptr;

void add_to_history(vector<string>& args);
string append_strings(const string& str1, const string& str2);
int display_history(vector<string>& args);
int get_num_builtins();
int change_directory(vector<string>& args);
int display_help(vector<string>& args);
int exit_shell(vector<string>& args);
int launch_shell(vector<string>& args);
int execute_shell(vector<string>& args);
string read_input_line();
vector<string> split_input_line(const string& line);

void add_to_history(vector<string>& args)
{
    shared_ptr<Node> ptr = make_shared<Node>();

    if (head == nullptr)
    {
        head = make_shared<Node>();
        head->command = "";

        string space = " ";

        if (!args.empty())
            head->command = append_strings(args[0], space);

        if (args.size() > 1)
            head->command = append_strings(head->command, args[1]);

        head->next = nullptr;
        cur = head;
    }
    else
    {
        ptr = make_shared<Node>();
        string space = " ";

        if (!args.empty())
            ptr->command = append_strings(args[0], space);

        if (args.size() > 1)
            ptr->command = append_strings(ptr->command, args[1]);

        cur->next = ptr;
        ptr->next = nullptr;
        cur = ptr;
    }
}

string append_strings(const string& str1, const string& str2)
{
    return str1 + str2;
}

int display_history(vector<string>& args)
{
    shared_ptr<Node> ptr = head;
    int i = 1;
    while (ptr != nullptr)
    {
        cout << " " << i++ << " " << ptr->command << endl;
        ptr = ptr->next;
    }
    return 1;
}

int get_num_builtins()
{
    return built_in_commands.size();
}

int change_directory(vector<string>& args)
{
    if (args.size() < 2)
    {
        cerr << "shell: expected argument to \"cd\"" << endl;
    }
    else
    {
        if (chdir(args[1].c_str()) != 0)
        {
            perror("shell");
        }
    }
    return 1;
}

int display_help(vector<string>& args)
{
    cout << "Type program names and arguments, and hit enter." << endl;
    cout << "The following are built-in commands:" << endl;

    for (const auto& command : built_in_commands)
    {
        cout << "  " << command << endl;
    }

    cout << "Use the man command for information on other programs." << endl;
    return 1;
}

int exit_shell(vector<string>& args)
{
    return 0;
}

int launch_shell(vector<string>& args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        vector<char*> c_args(args.size() + 1);
        for (size_t i = 0; i < args.size(); ++i)
        {
            c_args[i] = const_cast<char*>(args[i].c_str());
        }
        c_args[args.size()] = nullptr;

        if (execvp(c_args[0], c_args.data()) == -1)
        {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("shell");
    }
    else
    {
        // Parent process
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute_shell(vector<string>& args)
{
    if (args.empty())
    {
        // An empty command was entered.
        return 1;
    }

    for (size_t i = 0; i < get_num_builtins(); ++i)
    {
        if (args[0] == built_in_commands[i])
        {
            return (*built_in_functions[i])(args);
        }
    }

    return launch_shell(args);
}

string read_input_line()
{
    string line;
    if (!getline(cin, line))
    {
        if (cin.eof())
        {
            exit(EXIT_SUCCESS); // We received an EOF
        }
        else
        {
            perror("shell: getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

vector<string> split_input_line(const string& line)
{
    vector<string> tokens;
    size_t pos = 0;
    size_t found = 0;
    while ((found = line.find_first_of(SHELL_TOK_DELIM, pos)) != string::npos)
    {
        if (found != pos)
        {
            tokens.push_back(line.substr(pos, found - pos));
        }
        pos = found + 1;
    }
    if (pos < line.size())
    {
        tokens.push_back(line.substr(pos));
    }
    return tokens;
}

void shell_loop()
{
    string line;
    vector<string> args;
    int status = 1;

    do
    {
        cout << "> ";
        line = read_input_line();
        args = split_input_line(line);
        add_to_history(args);
        status = execute_shell(args);
    } while (status);
}

int main(int argc, char** argv)
{
    // Run the shell command loop.
    shell_loop();

    return EXIT_SUCCESS;
}
