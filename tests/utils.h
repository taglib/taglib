#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#endif
#include <stdio.h>
#include <string>

using namespace std;

inline string copyFile(const string &filename, const string &ext)
{
  string newname = string(tempnam(NULL, NULL)) + ext;
  string oldname = string("data/") + filename + ext;
#ifdef _WIN32
  CopyFile(oldname.c_str(), newname.c_str(), FALSE);
  SetFileAttributes(newname.c_str(), GetFileAttributes(newname.c_str()) & ~FILE_ATTRIBUTE_READONLY);
#else
  char buffer[4096];
  int bytes;
  int inf = open(oldname.c_str(), O_RDONLY);
  int outf = open(newname.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  while((bytes = read(inf, buffer, sizeof(buffer))) > 0)
    write(outf, buffer, bytes);
  close(outf);
  close(inf);
#endif
  return newname;
}

inline void deleteFile(const string &filename)
{
  remove(filename.c_str());
}

class ScopedFileCopy
{
public:
  ScopedFileCopy(const string &filename, const string &ext, bool deleteFile=true)
  {
    m_deleteFile = deleteFile;
    m_filename = copyFile(filename, ext);
  }

  ~ScopedFileCopy()
  {
    if(m_deleteFile)
      deleteFile(m_filename);
  }

  string fileName()
  {
    return m_filename;
  }

private:
  bool m_deleteFile;
  string m_filename;
};
