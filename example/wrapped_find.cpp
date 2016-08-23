#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include "untar/untar.h"

// Launch the resulting executable with the tar file in argument

using namespace untar;

int main(int argc, char **argv) {
								tarFile* file;
								++argv; /* Skip program name */
								if (*argv != NULL) {
																file = new tarFile(*argv, File);
								}

								// We now have all the files ready to be extracted.
								// If we want to find a file called Mommy.txt, use the map.find method
								// First define the key you are looking for. This is the filename in the tar
								// For example : myFolder/Mommy.txt
								string key = "myFolder/Mommy.txt";

								size_t start;
								int filesize;
								ifstream * filestream = file->find(key, &filesize, &start);
								if (filestream != nullptr) {
																if (filestream->is_open()) {
																								char dataoutput[500];
																								filestream->seekg(start, ios_base::beg);
																								filestream->read(dataoutput, filesize);
																								cout << string(dataoutput, filesize);
																}
								}

								system("pause");

								return (0);
}
