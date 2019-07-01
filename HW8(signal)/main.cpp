#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <ucontext.h>
#include <unistd.h>
#include <cmath>
#include <climits>
#include <sys/ucontext.h>

typedef long long LL;

void write_part(uint8_t byte) {
    char c = byte < 10 ? byte + '0' : byte + 'A' - 10;
    write(1, &c, 1);
}

void write_byte(uint8_t byte) {
    write_part(byte / 16);
    write_part(byte % 16);
}

void safe_write(uint64_t value) {
    for (int i = 7; i >= 0; --i) {
        write_byte(0xFF & (value >> (8 * i)));
    }
}

void safe_write(const char* string) {
    write(1, string, strlen(string));
}

const char* register_to_string(int reg) {
    switch (reg) {
        case 0:
            return "REG_R8";
        case 1:
            return "REG_R9";
        case 2:
            return "REG_R10";
        case 3:
            return "REG_R11";
        case 4:
            return "REG_R12";
        case 5:
            return "REG_R13";
        case 6:
            return "REG_R14";
        case 7:
            return "REG_R15";
        case 8:
            return "REG_RDI";
        case 9:
            return "REG_RSI";
        case 10:
            return "REG_RBP";
        case 11:
            return "REG_RBX";
        case 12:
            return "REG_RDX";
        case 13:
            return "REG_RAX";
        case 14:
            return "REG_RCX";
        case 15:
            return "REG_RSP";
        case 16:
            return "REG_RIP";
        case 17:
            return "REG_EFL";
        case 18:
            return "REG_CSGSFS";
        case 19:
            return "REG_ERR";
        case 20:
            return "REG_TRAPNO";
        case 21:
            return "REG_OLDMASK";
        case 22:
            return "REG_CR2";
        default:
            return "REG_UNKNOWN";
    }
}

void address_dump(int my_pipe[], char* address, int index) {
    uint8_t value;
    char* cur = address + index;
    if (write(my_pipe[1], cur, 1) < 0 || read(my_pipe[0], &value, 1) < 0) {
        safe_write("Unknown");
    } else {
        safe_write(value);
    }
    safe_write("\n");
}

void memory_dump(char* address) {
    int my_pipe[2];
    if (pipe(my_pipe) < 0) {
        safe_write("Error. Unable to pipe");
        _exit(EXIT_FAILURE);
    }
    safe_write("--------MEMORY--------\n");
    if (address == nullptr) {
        safe_write("NULL\n");
        _exit(EXIT_FAILURE);
    }
    for (int i = -20; i <= 20; ++i) {
        address_dump(my_pipe, address, i);
    }
}

void registers_dump(ucontext_t* context) {
    safe_write("--------REGISTERS--------\n");
    greg_t* gregs = context->uc_mcontext.gregs;
    for (int reg = 0; reg < NGREG; ++reg) {
        safe_write(register_to_string(reg));
        safe_write(": ");
        safe_write(gregs[reg]);
        safe_write("\n");
    }
}

void handler_sigsegv(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        safe_write("ERROR. Segmentation fault. Tried to access ");
        safe_write((uint64_t)siginfo->si_addr);
        safe_write("\n");
        switch (siginfo->si_code) {
            case SEGV_MAPERR:
                safe_write("Address not mapped to object.\n");
                break;
            case SEGV_ACCERR:
                safe_write("Invalid permissions for mapped object.\n");
                break;
            default:
                safe_write("Unknown error.\n");
        }
        registers_dump((ucontext_t*)context);
        memory_dump((char*)siginfo->si_addr);
    }
    _exit(EXIT_FAILURE);
}

void test1() {
    char* c = (char*)"LETSROCK";
    c[9] = 'A';
}

void test2() {
    int* x = nullptr;
    *x = 10;
}

int main(int argc, char* argv[]) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = handler_sigsegv;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        safe_write("Error. Sigaction failed\n");
        _exit(EXIT_FAILURE);
    }
    test1();
    //test2();
    _exit(EXIT_SUCCESS);
}