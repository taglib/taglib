#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>

using namespace std;

inline string testFilePath(const string &filename)
{
  return string(TESTS_DIR "data/") + filename;
}

#define TEST_FILE_PATH_C(f) testFilePath(f).c_str()

inline string copyFile(const string &filename, const string &ext)
{
  char testFileName[1024];

#ifdef _WIN32
  GetTempPathA(sizeof(testFileName), testFileName);
  GetTempFileNameA(testFileName, "tag", 0, testFileName);
  DeleteFileA(testFileName);
# if defined(_MSC_VER) && _MSC_VER > 1500
  strcat_s(testFileName, ext.c_str());
# else
  strcat(testFileName, ext.c_str());
# endif
#else
  snprintf(testFileName, sizeof(testFileName), "/%s/taglib-test-XXXXXX%s", P_tmpdir, ext.c_str());
  static_cast<void>(mkstemps(testFileName, 6));
#endif

  string sourceFileName = testFilePath(filename) + ext;
  ifstream source(sourceFileName.c_str(), std::ios::binary);
  ofstream destination(testFileName, std::ios::binary);
  destination << source.rdbuf();
  return string(testFileName);
}

inline void deleteFile(const string &filename)
{
  remove(filename.c_str());
}

inline bool fileEqual(const string &filename1, const string &filename2)
{
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  ifstream stream1(filename1.c_str(), ios_base::in | ios_base::binary);
  ifstream stream2(filename2.c_str(), ios_base::in | ios_base::binary);

  if(!stream1 && !stream2) return true;
  if(!stream1 || !stream2) return false;

  for(;;)
  {
    stream1.read(buf1, BUFSIZ);
    stream2.read(buf2, BUFSIZ);

    streamsize n1 = stream1.gcount();
    streamsize n2 = stream2.gcount();

    if(n1 != n2) return false;

    if(n1 == 0) break;

    if(memcmp(buf1, buf2, static_cast<size_t>(n1)) != 0) return false;
  }

  return stream1.good() == stream2.good();
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
