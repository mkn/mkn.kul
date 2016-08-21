/**
Copyright (c) 2013, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// This file is included by other files and is not in itself syntactically correct.

// void kul::cli::asArgs(const std::string& cmd, std::vector<std::string>& args){

    std::string arg;
    bool openQuotesS = false;
    bool openQuotesD = false;
    bool backSlashed = false;
    for(const char c : cmd){
        if(backSlashed){
            backSlashed = false;
            arg += c;
            continue;
        }
        switch(c){
            case ' ':
                if(!openQuotesD && !openQuotesS){               //     ||||| ||||| ||||| ||||| |||||
                    if(arg.size() > 0) args.push_back(arg);     //     ||    || || || || ||    ||
                    arg.clear();                                //     ||||| ||||| ||||| ||    |||||
                    continue;                                   //        || ||    || || ||    ||
                }                                               //     ||||| ||    || || ||||| |||||
                break;
            case '"':
                if(openQuotesD && !openQuotesS){
                    openQuotesD = false;
                    args.push_back(arg);
                    arg.clear();
                }else{ 
                    openQuotesD = true;
                }
                continue;
            case '\'':
                if(openQuotesS && !openQuotesD){
                    openQuotesS = false;
                    args.push_back(arg);
                    arg.clear();
                }else{ 
                    openQuotesS = true;
                }
                continue;
            case '\\':
                if(!openQuotesS && !openQuotesD){
                    backSlashed = true;
                    continue;
                }
                break;
        }
        arg += c;
    }
    if(arg.size() > 0) args.push_back(arg);
                    
// }