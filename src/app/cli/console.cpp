/*
 *    Copyright (c) 2019, The OpenThread Commissioner Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   The file implements Console.
 */

#include "app/cli/console.hpp"

#include <iostream>
#include <thread>
#include <time.h>
#include <sys/select.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>


namespace ot {

namespace commissioner {

bool gVerbose = false;
char *gInput; 
bool gReadlineActive = false;

std::string Console::mPrompt;

void Console::SetPrompt(const std::string &aPrompt)
{
    mPrompt = aPrompt;
}

void Console::ReadlineCallback(char* aInput) 
{ 
    gInput = aInput;

    if(gInput != nullptr)
    {
        if(strlen(gInput) > 0)
        {
            rl_callback_handler_remove();
            add_history(gInput);
        }
        else
        {
            gInput = nullptr;
        }
    }
}

std::string Console::Read()
{
    gInput = nullptr;

    gReadlineActive = true;
    rl_callback_handler_install((mPrompt + "> ").c_str(), ReadlineCallback);

    while (gInput == nullptr) 
    {
        struct timeval timeout = {0, kConsolePollPeriod * 1000};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &timeout);

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) 
        {
            rl_callback_read_char();
        } 
        else if (ret == 0) 
        {
            // do other work if needed
        } 
    }

    gReadlineActive = false;

    return gInput;
}

void Console::Write(const std::string &aLine, Color aColor)
{
    static const std::string kResetCode = "\u001b[0m";
    std::string              colorCode;
    char*                    savedLine;
    int                      savedPoint;


    if(gReadlineActive)
    {
        savedPoint = rl_point;
        savedLine = rl_copy_text(0, rl_end);
        rl_save_prompt();
        rl_replace_line("", 0);
        rl_redisplay();
    }

    switch (aColor)
    {
    case Color::kDefault:
        colorCode = "\u001b[0m";
        break;
    case Color::kWhite:
        colorCode = "\u001b[37m";
        break;
    case Color::kRed:
        colorCode = "\u001b[31m";
        break;
    case Color::kGreen:
        colorCode = "\u001b[32m";
        break;
    case Color::kYellow:
        colorCode = "\u001b[33m";
        break;
    case Color::kBlue:
        colorCode = "\u001b[34m";
        break;
    case Color::kMagenta:
        colorCode = "\u001b[35m";
        break;
    case Color::kCyan:
        colorCode = "\u001b[36m";
        break;
    }

    std::cout << colorCode << aLine << kResetCode << std::endl;

    if(gReadlineActive)
    {
        rl_restore_prompt();
        rl_replace_line(savedLine, 0);
        rl_point = savedPoint;
        rl_redisplay();
        free(savedLine);
    }
}

} // namespace commissioner

} // namespace ot
