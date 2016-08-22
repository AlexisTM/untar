Untar - c++ TAR extraction implementation 
=========

Another *simplified* c++ tar inflate implementation with *no dependency*. This implementation keeps the file open while the tarFile object is instantiated and gives you a map of tarEntry object which can each inflate themselves on demand. It is *your job* to create the files. This library only gives your the streams.

**Untar** has been developed to compress SFML assets into one main file.

Installation
---------

* Download one release
* Add untar/includes in the includes of your project or in your include dirs.
* Add `#include "untar/untar.h"` in your headers
* You are ready to go !

Pros/cons
---------

### Pros
* No dependency
* Extraction in memory (... only ...)
* Map of tarEntry
* map.find is O(log(N)), which is incredibly faster when you have many files than a list which is O(N)
* Keep the file open, ready to read
* Only read headers to be faster
* Checksum verification

### Cons
* No extraction on disk
* No compression (not needed in my project, if you need it, feel free to contribute !)
* You probably need to overload the final funciton (in tarEntry) to fits your needs
* Not final version, but usefull

Usage
---------

```cpp
#include "untar/untar.h"

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

    // Find the file itself
    auto it = file->entries.find(key);

    // If the file we found is NOT the end of the map (which means we did not find it), then extract the data !
    if (it != file->entries.end())
    {
        tarEntry * data = it->second;

        // Get the ifstream, startbyte and filesize in one call        
        size_t start;
        int filesize;
        ifstream * _tarfile = data->wantToExtract(&filesize, &start);

        // NOTE : file->_tarfile.is_open() is the same as the one in the entries.
        // tarFile::_tarfile will be private in future release, it is for debugging reason  

        // CARE THE OVERFLOW, ALWAYS
        char dataoutput[500];

        // always be sure the file is open before trying to read. Else we got shitty random data.
        if(_tarfile->is_open()){

           // Go to the beginning of the file and read the whole file
           _tarfile->seekg(data->getStartingByte(), ios_base::beg);

           // Read the data
           _tarfile->read(dataoutput, filesize);

           // Show the result
           cout << string(dataoutput, filesize);
        }
    }
    system("pause");

    return (0);
}
```

Documentation
---------

Waiting for a better documentation.

```cpp
namespace untar {

	using namespace std;

	// Type of file in the TAR documentation
	enum tarEntryType {
		FileCompatibilityType = '\0',
		FileType = '0',
		HardlinkType = '1',
		SymlinkType,
		CharacterDeviceType,
		BlockDeviceType,
		DirType,
		FifoType,
		ReservedType,
		OtherType
	};

	// Limit entries to Files or Dir to limit the map size
	enum tarMode {
		File = 1,
		Hardlink = 2,
		Symlink = 4,
		CharacterDevice = 8,
		BlockDevice = 16,
		Dir = 32,
		Fifo = 64,
		Reserved = 128,
		All = 255
	};

	// The tarEntry class represents an ENTRY inside a TAR file. It can be a File, Dir, Symlink, ... Important parts to be able to use it :
	// tarEntry::tarEntry(string filename, int filesize, size_t startOfFile, tarEntryType type, string parentTarFilename, ifstream * _tarfile);
	// ifstream * tarEntry::wantToExtract(int * filesize, size_t * startInMemory);
	// string getFilename();
	// size_t getStartingByte();
	class tarEntry {
		friend class tarFile;
	public :
		// To be able to create null tarEntry
		tarEntry();
		// Default constructor, prefer this one
		tarEntry(string filename, int filesize, size_t startOfFile, tarEntryType type, string parentTarFilename, ifstream * _tarfile);
		// Constructor to avoid error for people who failed the instantiation. Could disapear when I will have properly documented the library.
		tarEntry(tarEntry const & cpy);
		// Destructor
		~tarEntry();
		// Get the tar filename where this file comes from
		string getParentFilename();
		// Get the file size
		int getFileSize();
		// Get the filename (containing the path)
		string getFilename();
		// Get the starting byte in the stream
		size_t getStartingByte();
		// Get the tar type. Is it a DirType, FileType, SymlinkType ?
		tarEntryType getType();
		// Get all usefull data we need to extract our data in one call
		ifstream * tarEntry::wantToExtract(int * filesize, size_t * startInMemory);

	private:
		// The stream, this is the same as the one of the tarFile object. Opened by default.
		std::ifstream * _tarfile;
		// If the file is extacted, set to true.
		// For future usage, in my project in SFML
		bool _extracted;
		// Starting byte in the tar file (stream)
		size_t _startOfFile;
		// Filesize
		int _filesize;
		// Filename (containing the path)
		string _filename;
		// Type of the entry
		tarEntryType _type;
		// What is my dad TAR ?
		string  _parentTarFilename;
	};

	// tarFile represents a TAR FILE opened. Important parts to be able to use it :
	// tarFile::tarFile(char * filename, int filter = All);
	// static map<string, tarEntry *> tarFile::entries
	class tarFile {
		friend class tarEntry;
	public:
		// If you don't give a filename, don't forget to initiate via tarFile::open(...)  
		tarFile();
		// Default initiation
		tarFile(char * filename, int filter = All);
		// The destructor
		~tarFile();

		// Open a file in case you didn't instanciated the class with a filename
		void open(char * filename, int filter = All);
		// Get the filename of the opened file
		string getFilename();
		// Map of tarEntries, containing all the files
		// TODO
		// Put entries as private and wrap the find function in the class
		static map<string, tarEntry *> entries;
		// This is the tar filestream
		static std::ifstream _tarfile;

	private:
		// Read the file and store entries
		void getAllEntries(int filter = All);
		// Add a new tarEntry into the map (entries)
		void tarFile::addEntryToMap(string filename, int filesize, tarEntryType type);
		// Check if the header is Null. The tar file ends on two Null headers
		bool isNullHeader(const char *p);
		// Verify the checksum, actually, it stops the reading on error
		static int verifyChecksum(const char *p);
		// Used to read the header
		static int parseoct(const char *p, size_t n);
		// Did we get entries ? If yes, don't get them again
		bool _get_all_entries;
		// Remember where comes the data form
		char * _filename;
	};

}
```

Inspiration
---------

* [ctar from libarchive](https://github.com/libarchive/libarchive/blob/master/contrib/untar.c)
* [tarlib](http://www.codeproject.com/Articles/470999/tarlib-Windows-TAR-Library)

Todo
---------

* Error handling
* Add functions to extract to system files directly
* Compression

Contributing
---------

* Fork the project
* Make a branch
* Edit the branch
* Make a pull request from that branch

Credits
---------

Alexis Paques (@AlexisTM)
