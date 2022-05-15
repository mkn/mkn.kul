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
// IWYU pragma: private, include "mkn/kul/os.hpp"

std::vector<mkn::kul::File> mkn::kul::Dir::files(bool recursive) const KTHROW(fs::Exception) {
  if (!is()) KEXCEPT(fs::Exception, "Directory : \"" + path() + "\" does not exist");

  std::vector<File> fs;
  DIR *dir = opendir(path().c_str());
  struct dirent *entry = readdir(dir);
  while (entry != NULL) {
    if (!mkn::kul::Dir(JOIN(real(), entry->d_name)).is()) fs.push_back(File(entry->d_name, *this));
    entry = readdir(dir);
  }
  closedir(dir);
  if (recursive) {
    for (mkn::kul::Dir const &d : dirs()) {
      const std::vector<mkn::kul::File> &tFs = d.files(true);
      fs.insert(fs.end(), tFs.begin(), tFs.end());
    }
  }
  return fs;
}
