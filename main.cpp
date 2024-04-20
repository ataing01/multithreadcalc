#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>

#define MAX_DIGITS 10

//struct to store input variables
struct inputvar {
    char var;
    int num;
};

struct operation {
    std::string first;
    std::string second;
    char op = ' ';
};

int getval(std::vector<inputvar*> v, char a) {
    for (int i = 0; i < v.size(); i++) {
        if (v[i]->var == a) {
            return v[i]->num;
        }
    }
    return -1;
}

//makeshift getline()
void gline(char* argv, std::string &s) {
    std::string res = "";
    int i = 0;
    if (s.length() != 0) {
        i = s.length();
        for (int j = 0; j <= strlen(argv); j++) {
            std::string test = "";
            int h;
            for (h = j; h < j+s.length(); h++) {
                test += argv[h];
            }
            if (test == s) {
                i = h;
                break;
            }
        }
        while (i <= strlen(argv) && (argv[i] == '\n' || argv[i] == '\r')) {
            i++;
        }
    }
    while (i <= strlen(argv) && argv[i] != '\n' && argv[i] != '\r') {
        res += argv[i];
        i++;
    }
    s = res;
}

//check if character is an operator
bool isoperator(char c) {
    if (c == '+' ||
        c == '-' ||
        c == '*' ||
        c == '/') {
            return true;
        }
    return false;
}

//int to char
char* inttochar(int i) {
    char res[MAX_DIGITS + sizeof(char)];
    std::snprintf(res, sizeof(res), "%d", i);
    return res;
}

//performs integer math
char* domath(char op1, char op2, char oper) {
    //way to convert char to int
    int n1 = op1 - '0';
    int n2 = op2 - '0';
    int res;
    if (oper == '+') {
        res = n1+n2;
    }
    else if (oper == '-') {
        res = n1-n2;
    }
    else if (oper == '*') {
        res = n1*n2;
    }
    else if (oper == '/') {
        res = n1/n2;
    }
    else {
        fprintf(stderr, "it's joever");
    }
    char* cres = inttochar(res);
    return cres;
}

//remove spaces and semicolons from string
void nospace(std::string &s) {
    std::string ph; //placeholder string
    for (int i = 0; i < s.length(); i++) {
        if (s[i] != ' ' && s[i] != ';') {
            ph += s[i];
        }
    }
    s = ph;
}

//given a->p0, return operator, relevant objects and whether or not objects are processes
//is object is num then it is a process, letters are input vars
operation* parse(std::string s) {
    std::string one = "";
    std::string two = "";
    nospace(s);
    int i = 0;
    while (!(s[i] == '-' && s[i+1] == '>')) {
        one += s[i];
        i++;
    }
    two = s.substr(i+2);
    operation* res = new operation;
    if (isoperator(s.front())) {
        res->op = s.front();
        one = one.substr(1); //cut op from 'one'
    }
    if (one.front() == 'p' && isdigit(one.back())) {
        one = one.substr(1);
    }
    if (two.front() == 'p' && isdigit(two.back())) {
        two = two.substr(1);
    }
    res->first = one;
    res->second = two;
    return res;
}

int main(int argc, char** argv) {
    /*
    ./calc "$(<df.txt)" "$(<input.txt)"
    */
    //get inputs and dataflow
    std::string inputs;
    gline(argv[2], inputs);
    std::string df;
    gline(argv[1], df);

    //get num of input vars
    int ivnum = 1;
    for (int i = 0; i < inputs.length(); i++) {
        if (inputs[i] == ',') {
            ivnum++;
        }
    }

    std::vector<inputvar*> ivs;
    pid_t pid[2];
    int fd1[2];
    int fd2[2];
    pipe(fd1); pipe(fd2);

    //get input vars
    for (int h = 0; h < ivnum; h++) {
        if ((pid[0] = fork()) == 0) {
            int t1 = 0;
            int t2 = t1;
            int val[1] = {-1};
            for (int i = 0; i < inputs.length(); i++) {
                std::cout << inputs[i] << std::endl;
                if (isdigit(inputs[i])) {
                    t1 = i;
                    while (i < inputs.length() && isdigit(inputs[i])) {
                        i++;
                    }
                    t2 = i;
                    val[0] = stoi(inputs.substr(t1, t2-t1));
                    write(fd1[1], val, sizeof(val));
                }
            }
            exit(0);
        }
        else if ((pid[1] = fork()) == 0) {
            char var[1] = {' '};
            for (int j = 0; j < df.length(); j++) {
                std::cout << df[j] << std::endl;
                if (df[j] == ',' || df[j] == ' ') {
                    var[0] = df[j+1];
                    write(fd2[1], var, sizeof(var));
                }
            }
            exit(0);
        }
        else {
            int _val[1];
            read(fd1[0], _val, sizeof(_val));

            char _var[1];
            read(fd2[0], _var, sizeof(_var));

            inputvar* iv = new inputvar;
            iv->num = _val[0];
            iv->var = _var[0];
            ivs.push_back(iv);
        }
    }
    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);

    //get num of internal vars
    gline(argv[1], df);
    int processnum = 0;
    for (int i = 0; i < df.length(); i++) {
        if (df[i] == ',') {
            processnum++;
        }
    }
    processnum++;

    //get operations
    std::vector<operation*> ops;
    gline(argv[1], df);
    int c = 1;
    while (df.substr(0,5) != "write") {
        std::cout << "line " << c << std::endl;
        ops.push_back(parse(df));
        gline(argv[1], df);
        c++;
    }

    pid_t pids[processnum];
    int fd[processnum*2]; //maybe close old fds
    for (int i = 0; i < processnum; i++) {
        pipe(&fd[2*i]);
    }
    std::vector<char> res(processnum, NULL);

    //everything else
    //print ivs
    for (int i = 0; i < ivs.size(); i++) {
        std::cout << ivs[i]->var << ": " << ivs[i]->num << std::endl;
    }

    //print ops
    for (int i = 0; i < ops.size(); i++) {
        std::cout << ops[i]->first << " " << ops[i]->op << " " << ops[i]->second << std::endl;
    }

    //handle ops
    for (int i = 0; i < ops.size(); i++) {
        std::cout << "i: " << i << std::endl;
        //pnum = process num
        int pnum = stoi(ops[i]->second.substr(ops[i]->second.length()-1, 1));
        std::cout << "pnum: " << pnum << std::endl;
        //if child
        if ((pids[pnum] = fork()) == 0) {
            std::cout << "this is the child" << std::endl;
            char val[1];
            //if process is empty, then fill it
            if (res[pnum] == NULL) {
                val[0] = ops[i]->first[0];
                //if digit then get val from p[val]
                if (isdigit(val[0])) {
                    int idx = val[0] - '0';
                    val[0] = res[idx];
                }
                std::cout << "p" << pnum << " is empty, writing " << val[0] << std::endl;
                write(fd[pnum*2+1], val, sizeof(val));
                std::cout << "writing successful" << std::endl;
            }
            //if process is not empty, perform math
            else {
                std::cout << "doing math" << std::endl;
                std::cout << ops[i]->first << ops[i]->op << ops[i]->second << std::endl;
                char first;
                char second;
                //parse ops to convert chars/ints to the actual numbers to be used
                //if op is digit, get res from p[]
                //if res is not digit, then getval() from ivs
                if (isdigit(ops[i]->first[0])) {
                    first = res[stoi(ops[i]->first)];
                    if (isdigit(first) == false) {
                        first = *inttochar(getval(ivs, first));
                    }
                }
                //if op is not digit then getval() from ivs
                else {
                    first = *inttochar(getval(ivs, ops[i]->first[0]));
                }
                if (isdigit(ops[i]->second[0])) {
                    second = res[stoi(ops[i]->second)];
                    if (isdigit(second) == false) {
                        second = *inttochar(getval(ivs, second));
                    }
                }
                else {
                    second = *inttochar(getval(ivs, ops[i]->second[0]));
                }
                std::cout << first << " " << ops[i]->op << " " << second << std::endl;
                val[0] = *domath(second, first, ops[i]->op);
                write(fd[pnum*2+1], val, sizeof(val));
            }
            exit(0);
        }    
        //if parent
        else {
            std::cout << "this is not the child" << std::endl;
            char val[1];
            std::cout << "reading" << std::endl;
            read(fd[pnum*2], val, sizeof(val));
            std::cout << "reading successful, got " << val[0] << std::endl;
            res[pnum] = val[0];
            std::cout << "inserted " << res[pnum] << std::endl;
        }
    }

    std::cout << "end" << std::endl;

    for (int i = 0; i < processnum; i++) {
        std::cout << res[i] << std::endl;
    }
}