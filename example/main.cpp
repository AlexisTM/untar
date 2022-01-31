#include "untar/untar.h"
#include <iostream>
#include <stdio.h>


// Launch the resulting executable with the tar file in argument, like :
// main.exe example.tar

int main(int argc, char **argv) {
  untar::tarFile *file;
  ++argv; /* Skip program name */
  if (*argv != NULL) {
    file = new untar::tarFile(*argv, untar::File);
  }

  // We now have all the files ready to be extracted.
  // If we want to find a file called Mommy.txt, use the map.find method
  // First define the key you are looking for. This is the filename in the tar
  // For example : myFolder/Mommy.txt
  std::string key("myFolder/Mommy.txt");

  // Find the file itself
  auto it = file->entries.find(key);

  // If the file we found is NOT the end of the map (which means we did not find
  // it), then extract the data !
  if (it != file->entries.end()) {
    untar::tarEntry *data = it->second;

    // Get the ifstream, startbyte and filesize in one call
    std::size_t start;
    int filesize;
    std::ifstream *_tarfile = data->wantToExtract(&filesize, &start);

    // NOTE : file->_tarfile.is_open() is the same as the one in the entries.
    // tarFile::_tarfile will be private in future release, it is for debugging
    // reason

    // CARE THE OVERFLOW, ALWAYS
    char dataoutput[500];

    // always be sure the file is open before trying to read. Else we got shitty
    // random data.
    if (_tarfile->is_open()) {

      // Go to the beginning of the file and read the whole file
      _tarfile->seekg(data->getStartingByte(), std::ios_base::beg);

      // Read the data
      _tarfile->read(dataoutput, filesize);

      // Show the result
      std::cout << std::string(dataoutput, filesize) << std::endl;
    }
  }
  delete file;
  system("pause");

  return (0);
}
