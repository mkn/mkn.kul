/**
Copyright (c) 2024, Philip Deegan.
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

std::vector<mkn::kul::File> mkn::kul::Dir::files(bool recursive) const KTHROW(fs::Exception) {
  if (!is()) KEXCEPT(fs::Exception, "Directory : \"" + path() + "\" does not exist");

  std::vector<File> fs;
  WIN32_FIND_DATA fdFile;
  HANDLE hFind = NULL;
  char sPath[2048];
  sprintf_s(sPath, "%s\\*.*", path().c_str());
  if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    KEXCEPT(fs::Exception, "Directory : \"" + path() + "\" does not exist");

  do {
    if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0) {
      sprintf_s(sPath, "%s\\%s", path().c_str(), fdFile.cFileName);
      if (!(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        std::string f(sPath);
        fs.push_back(File(f.substr(f.rfind(mkn::kul::Dir::SEP()) + 1), *this));
      }
    }
  } while (FindNextFile(hFind, &fdFile));
  FindClose(hFind);
  if (recursive) {
    for (Dir const& d : dirs()) {
      std::vector<File> tFiles = d.files(true);
      fs.insert(fs.end(), tFiles.begin(), tFiles.end());
    }
  }
  return fs;
}
