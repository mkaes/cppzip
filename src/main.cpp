#include <cppzip/zip_archive.h>
#include <cppzip/zip_entry.h>
#include <fstream>
#include <iostream>
#include <zlib.h>

int main(int argc, char* argv[])
{
  std::cout << "Running: " << argv[0] << " ...\n";
  {
    cppzip::ZipArchive z;
    const std::string content{"TestData"};
    const auto res = z.addData("foobar/test.txt", content.c_str(), content.size());
    if (!res)
    {
      std::cout << "Add Data failed\n";
      return 1;
    }

      std::ofstream f("testzip.zip", std::ios::out | std::ios::binary);
    z.writeArchive(f);
  }
  return 0;
}
