/**
Copyright (c) 2026, Philip Deegan.
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
#ifndef MKN_KUL_OS_WIN_OS_BOT_HPP_
#define MKN_KUL_OS_WIN_OS_BOT_HPP_

void mkn::kul::Dir::rm() const {
  if (is()) {
    for (auto const& a : files()) a.rm();
    for (auto const& a : dirs()) a.rm();
    _rmdir(path().c_str());
  }
}

std::string mkn::kul::Dir::ESC(std::string s) {
  ESC_REPLACE(s, "\\", "\\\\");
  return s;
}

std::string mkn::kul::Dir::LOCL(std::string s) {
  mkn::kul::String::REPLACE_ALL(s, "/", "\\");
  return s;
}

bool mkn::kul::Dir::is() const {
  if (path().empty()) return false;
  DWORD ftyp = GetFileAttributesA(path().c_str());
  return (ftyp != INVALID_FILE_ATTRIBUTES && ftyp & FILE_ATTRIBUTE_DIRECTORY);
}
bool mkn::kul::Dir::mk() const {
  if (path().empty()) return false;
  mkn::kul::Dir const& prnt(parent());
  if (_p != prnt.path() && !prnt.is()) parent().mk();
  return CreateDirectory(locl().c_str(), NULL);
}
bool mkn::kul::Dir::root() const { return is() && real().size() == 3; }

bool mkn::kul::File::is() const {
  return !name().empty() && (bool)std::ifstream(_d.join(_n).c_str());
}
bool mkn::kul::File::rm() const {
  if (is()) {
    _unlink(_d.join(_n).c_str());
    return true;
  }
  return false;
}

bool mkn::kul::File::mk() const {
  FILE* pFile;
  fopen_s(&pFile, full().c_str(), "w");
  if (pFile != NULL) {
    fclose(pFile);
  }
  return pFile != NULL;
}

uint64_t mkn::kul::File::size() const {
  uint64_t r = 0;
  WIN32_FIND_DATA ffd;
  HANDLE h = FindFirstFile(TEXT(mini().c_str()), &ffd);
  if (h) {
    ULARGE_INTEGER ul;
    ul.HighPart = ffd.nFileSizeHigh;
    ul.LowPart = ffd.nFileSizeLow;
    r = ul.QuadPart;
    FindClose(h);
  }
  return r;
}

std::vector<mkn::kul::Dir> mkn::kul::Dir::dirs(bool incHidden) const KTHROW(fs::Exception) {
  if (!is()) KEXCEPT(fs::Exception, "Directory : \"" + path() + "\" does not exist");
  std::vector<Dir> dirs;

  WIN32_FIND_DATA fdFile;
  HANDLE hFind = NULL;
  char sPath[2048];
  sprintf_s(sPath, "%s\\*.*", path().c_str());
  if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    KEXCEPT(fs::Exception, "Directory : \"" + path() + "\" does not exist");
  do {
    if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0) {
      sprintf_s(sPath, "%s\\%s", path().c_str(), fdFile.cFileName);
      if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (!incHidden && std::string(sPath)
                                  .substr(std::string(sPath).rfind(mkn::kul::Dir::SEP()) + 1)
                                  .substr(0, 1)
                                  .compare(".") == 0)
          continue;
        dirs.push_back(Dir(sPath));
      }
    }
  } while (FindNextFile(hFind, &fdFile));
  FindClose(hFind);

  return dirs;
}

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

std::string mkn::kul::Dir::REAL(std::string const& s) KTHROW(fs::Exception) {
  char* expanded = _fullpath(NULL, s.c_str(), _MAX_PATH);
  if (expanded) {
    std::string dir(expanded);
    free(expanded);
    if (dir.size() && dir[dir.size() - 1] == '\\') dir.pop_back();
    return dir;
  }
  KEXCEPT(fs::Exception, "Item: \"" + s + "\" does not exist");
}

std::optional<std::string> mkn::kul::Dir::REAL_OR_NULL(std::string const& s) KTHROW(fs::Exception) {
  char* expanded = _fullpath(NULL, s.c_str(), _MAX_PATH);
  if (expanded) {
    std::string dir(expanded);
    free(expanded);
    if (dir.size() && dir[dir.size() - 1] == '\\') dir.pop_back();
    return dir;
  }
  return std::nullopt;
}

namespace mkn {
namespace kul {
namespace os {

inline uint16_t exec(std::string const& cmd, bool q = false) {
  if (q) {
    return system(std::string(cmd + " > nul").c_str());
  }
  return system(cmd.c_str());
}
}  // namespace os

namespace user {

inline mkn::kul::Dir home() {
  std::string const h(env::GET("HOME"));
  if (h.size()) return mkn::kul::Dir(h);
  return mkn::kul::Dir(std::string(env::GET("HOMEDRIVE")) + std::string(env::GET("HOMEPATH")));
}
inline mkn::kul::Dir home(std::string const& app) { return mkn::kul::Dir(home().join(app)); }

}  // namespace user

namespace env {
inline bool CWD(mkn::kul::Dir const& d) { return _chdir(d.path().c_str()) != -1; }
}  // namespace env
}  // namespace kul
}  // namespace mkn

#endif /* MKN_KUL_OS_WIN_OS_BOT_HPP_ */
