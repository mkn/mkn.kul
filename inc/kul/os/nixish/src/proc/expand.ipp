/**
Copyright (c) 2017, Philip Deegan.
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

void kul::Process::expand(std::string& s) const {
  std::string r = s;
  auto lb = s.find("$(");
  auto clb = s.find("\\$(");
  while ((lb - clb + 1) == 0) {
    lb = r.find("$(", clb + 3);
    clb = r.find("\\$(", clb + 3);
  }
  if (lb == std::string::npos) return;
  auto rb = s.find(")");
  auto crb = s.find("\\)");
  while ((rb - crb + 1) == 0) {
    rb = r.find(")", crb + 2);
    crb = r.find("\\)", crb + 2);
  }
  if (rb == std::string::npos) return;

  std::string k(r.substr(lb + 2, rb - 2 - lb));
  std::vector<std::string> cli(kul::cli::asArgs(k));
  std::stringstream ss;
  if (cli.size() > 1) {
    kul::Process p(cli[0]);
    kul::ProcessCapture pc(p);
    for (size_t i = 1; i < cli.size(); i++) p.arg(cli[i]);
    p.start();
    std::string out(pc.outs());
    if (*out.rbegin() == '\n') out.pop_back();
    std::string t(r.substr(0, lb) + out + r.substr(rb + 1));
    ss << r.substr(0, lb) << out << r.substr(rb + 1);
  } else
    ss << r.substr(0, lb) << kul::env::GET(cli[0].c_str()) << r.substr(rb + 1);

  std::string t(ss.str());
  expand(t);
  s = t;
}
