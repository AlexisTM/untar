#include <stdio.h>
#include <iostream>
#include "untar/untar.h"

// Launch the resulting executable with the tar file in argument

int main(int argc, char** argv)
{
    untar::tarFile* file;
    ++argv; /* Skip program name */
    if (*argv != NULL) {
        file = new untar::tarFile(*argv, untar::File);
    }

    // We now have all the files ready to be extracted.
    // If we want to find a file called Mommy.txt, use the map.find method
    // First define the key you are looking for. This is the filename in the tar
    // For example : myFolder/Mommy.txt
    std::string key = "myFolder/Mommy.txt";

    std::size_t start;
    int filesize;
    std::ifstream* filestream = file->find(key, &filesize, &start);
    if (filestream != nullptr) {
        if (filestream->is_open()) {
            char dataoutput[500];
            filestream->seekg(start, std::ios_base::beg);
            filestream->read(dataoutput, filesize);
            std::cout << std::string(dataoutput, filesize) << std::endl;;
        }
    }

    delete file;
    system("pause");

    return (0);
}
