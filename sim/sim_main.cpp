// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <memory>
#include <chrono>
#include <deque>
#include <fstream>
#include <sstream>

#include <fcntl.h>
#include <termios.h>

#include <verilated.h>
#include <iostream>

// Include model header, generated from Verilating "top.v"
#include "Vtop.h"

double sc_time_stamp()
{
    return 0.0;
}

static bool raw_mode = false;
static struct termios orig_termios;

void disable_raw_mode(int fd) {
    if (raw_mode) {
        tcsetattr(fd,TCSAFLUSH,&orig_termios);
        raw_mode = false;
        printf("Raw mode disabled.\n");
    }
}

void exit_handler(void) {
    disable_raw_mode(STDIN_FILENO);
}

int enable_raw_mode(int fd) {
    struct termios raw;
    int flags;

    if (raw_mode) return 0;
    if (!isatty(STDIN_FILENO)) goto fatal;
    atexit(exit_handler);
    if (tcgetattr(fd,&orig_termios) == -1) goto fatal;

    raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // Put terminal in raw mode after flushing
    if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;

    // Set to non-blocking mode
    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    raw_mode = true;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}


int main(int argc, char **argv, char **env)
{
    enable_raw_mode(STDIN_FILENO);

    // Create logs/ directory in case we have traces to put under it
    Verilated::mkdir("logs");

    // Construct a VerilatedContext to hold simulation time, etc.
    // Multiple modules (made later below with Vtop) may share the same
    // context to share time, or modules may have different contexts if
    // they should be independent from each other.

    // Using unique_ptr is similar to
    // "VerilatedContext* contextp = new VerilatedContext" then deleting at end.
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};

    // Set debug level, 0 is off, 9 is highest presently used
    // May be overridden by commandArgs argument parsing
    contextp->debug(0);

    // Randomization reset policy
    // May be overridden by commandArgs argument parsing
    contextp->randReset(0);

    // Verilator must compute traced signals
    contextp->traceEverOn(true);

    // Pass arguments so Verilated code can see them, e.g. $value$plusargs
    // This needs to be called before you create any model
    contextp->commandArgs(argc, argv);

    // Construct the Verilated model, from Vtop.h generated from Verilating "top.v".
    // Using unique_ptr is similar to "Vtop* top = new Vtop" then deleting at end.
    // "TOP" will be the hierarchical name of the module.
    const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};

    std::deque<uint8_t> keys;
    std::array<uint8_t, 1*1024*1024> sdc_data;
    uint32_t sdc_addr = 0;

    // Set Vtop's input signals
    top->i_rst = 1;

    bool done = false;
    int exit_value = 0;
    while (!contextp->gotFinish() && !done)
    {
        int nread;
        char c;
        nread = read(STDIN_FILENO, &c, 1);
        if (nread > 0) {
            if (c == 3) {
                // CTRL-C - ETX
                done = true;
            } else {
                keys.emplace_front(c);
            }
        }

        top->clk = !top->clk;

        // if posedge clk
        if (top->clk) {

            if (contextp->time() < 5)
            {
                top->i_rst = 1; // Assert reset
            }
            else
            {
                top->i_rst = 0; // Deassert reset
            }

            if (top->o_ext_addr == 0) {
                // exit
                if (top->o_ext_stb) {
                    if (top->o_ext_we != 0) {
                        exit_value = top->o_ext_dat_w;
                        done = true;
                    }
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }                
            } else if (top->o_ext_addr == 4) {
                // put char
                if (top->o_ext_stb) {
                    if (top->o_ext_we != 0) {
                        int c = top->o_ext_dat_w;
                        write(STDOUT_FILENO, &c, 1);
                    }
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }
            } else if (top->o_ext_addr == 8) {
                // read char
                if (top->o_ext_stb) {
                    uint8_t key = 0;

                    if (keys.size() > 0) {
                        key = keys.back();
                        keys.pop_back();
                    }
                    top->i_ext_dat_r = key;
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }
            } else if (top->o_ext_addr == 0xC || top->o_ext_addr == 0x10) {
                // SD card
                if (top->o_ext_stb) {
                    if (top->o_ext_we != 0) {
                        if (top->o_ext_addr == 0xC) {
                            sdc_addr = top->o_ext_dat_w;
                        } else {
                            sdc_data.at(sdc_addr) = (uint8_t)top->o_ext_dat_w;
                            sdc_addr++;
                        }
                    } else {
                        if (top->o_ext_addr == 0x10) {
                            top->i_ext_dat_r = sdc_data.at(sdc_addr);
                            sdc_addr++;
                        }
                    }
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }
            } else if (top->o_ext_addr >= 0xF000) {
                top->i_ext_dat_r = 0;
                if (top->o_ext_stb) {
                    if (top->o_ext_we != 0)
                        printf("LED: %02x\r\n", top->o_ext_dat_w);
                    top->i_ext_ack = 1;
                } else {
                    top->i_ext_ack = 0;
                }
            }
        }

        contextp->timeInc(1);
        top->eval();
    }

    // Final model cleanup
    top->final();

    printf("Exiting with value %d...\r\n", exit_value);
    exit(exit_value);
}
