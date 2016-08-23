#include "untar.h"

namespace untar {
	using namespace std;

	tarEntry::tarEntry()
	{

	}
	
	tarEntry::tarEntry(string filename, int filesize, size_t startOfFile, tarEntryType type, string parentTarFilename, ifstream * tarfile)
	{
		_tarfile = tarfile;
		_extracted = false;
		_filename = string(filename);
		_filesize = filesize;
		_startOfFile = startOfFile;
		_type = type;
		_parentTarFilename = parentTarFilename;
	}

	tarEntry::~tarEntry()
	{
		delete this;
	}

	tarEntry::tarEntry(tarEntry const & cpy)
	{
		tarEntry(cpy._filename, cpy._filesize, cpy._startOfFile, cpy._type, cpy._parentTarFilename, cpy._tarfile);
	}

	string tarEntry::getParentFilename()
	{
		return _parentTarFilename;
	}

	int tarEntry::getFileSize()
	{
		return _filesize;
	}

	string tarEntry::getFilename()
	{
		return string(_filename);
	}

	size_t tarEntry::getStartingByte()
	{
		return _startOfFile;
	}

	tarEntryType tarEntry::getType()
	{
		return tarEntryType();
	}

	ifstream* tarEntry::wantToExtract(int * filesize, size_t * startInMemory)
	{
		if (_tarfile != nullptr) {
			_tarfile->seekg(_startOfFile, ios_base::beg);
		}
		*filesize = _filesize;
		*startInMemory = _startOfFile;
		return _tarfile;
	}

	tarFile::tarFile()
	{
	}

	tarFile::tarFile(char * filename, int filter)
	{
		open(filename, filter);
	}

	tarFile::~tarFile()
	{
		// TODO
		// Do I have to delete every entry of the map or does map.clear() clean it ?
		entries.clear();
		_tarfile.close();
	}

	tarEntry * tarFile::find(string filename)
	{
		auto it = entries.find(filename);
		
		if (it == entries.end())
			return nullptr;

		return it->second;
	}

	ifstream * tarFile::find(string filename, int * filesize, size_t * start)
	{
		tarEntry * myEntry = find(filename);
		if (myEntry == nullptr) {
			*filesize = 0;
			*start = 0;
			return nullptr;
		}
		return myEntry->wantToExtract(filesize, start);
	}

	void tarFile::open(char * filename, int filter)
	{
		_get_all_entries = false;
		_filename = filename;
		_tarfile.open(_filename, ios_base::in | ios_base::binary);
		if (!_tarfile.is_open()) {
			// TODO
			// Error Handling 
			//cout << "Unable to open file : " << _filename << "\n";
		}
		else {
			getAllEntries(filter);
		}
	}

	string tarFile::getFilename()
	{
		return _filename;
	}

	void tarFile::addEntryToMap(string filename, int filesize, tarEntryType type) {
		tarEntry *entry = new tarEntry(filename, filesize, _tarfile.tellg(), type, _filename, &_tarfile);
		entries.insert(std::pair<string, tarEntry *>(filename, entry));
	}

	// The tarMode::filter gives the ability to choose the files to see.
	// If you want Files & Directories, you should use :
	// getAllEntries(tarMode::Dir || tarMode::File)
	// With the namespace, it becomes getAllEntries(Dir || File)
	// Which is easily readable.
	void tarFile::getAllEntries(int filter)
	{
		if (_tarfile.is_open()) {
			// TODO
			// Keep an eye on bytes we read. Error handling to be implemented
			size_t bytes_read = 0;

			// Go to beginning of the file, just to be sure
			_tarfile.seekg(0);

			char buff[512];
			do {
				size_t nextEntry = 0;
				// read the header
				//_tarfile.read(&buff[0], 512);
				_tarfile.read(buff, 512);
				bytes_read += _tarfile.gcount();
				
				// EndOfFile
				if (isNullHeader(buff))
					return;

				// Checksum verification
				if (!verifyChecksum(buff)) {
					// TODO
					// Error handling to be implemented
					return;
				}
				
				// read the filesize at buff[124], 12 bytes
				int filesize = parseoct(&buff[0] + 124, 12);

				// The actual size on disk of the file is a multiple of 512 in tar format
				nextEntry = 0;
				if (filesize) // filesize > 0
				{
					if (filesize % 512 == 0) {
						nextEntry = filesize;
					}
						
					else {
						nextEntry = (512 * (1 + filesize / 512));
					}
				}
				
				tarEntryType type = static_cast<tarEntryType>(buff[156]);

				// filename is \0 terminated... only if 99- chars long ! 
				// So add a \0 as the 101th char for 100 char long filename. (no trim, faster, safer and nastier)
				buff[100] = '\0';
				string filename = string(buff);

				if (filter == All) {
					addEntryToMap(filename, filesize, type);
				} else {
					// TODO : Need a good way to do this
					switch (type) {
					case FileCompatibilityType:
					case FileType:
						if (filter & tarMode::File)
							addEntryToMap(filename, filesize, type);
						break;
					case HardlinkType:
						if (filter & tarMode::Hardlink)
							addEntryToMap(filename, filesize, type);
						break;
					case SymlinkType:
						if (filter & tarMode::Symlink)
							addEntryToMap(filename, filesize, type);
						break;
					case CharacterDeviceType:
						if (filter & tarMode::CharacterDevice)
							addEntryToMap(filename, filesize, type);
						break;
					case BlockDeviceType:
						if (filter & tarMode::BlockDevice)
							addEntryToMap(filename, filesize, type);
						break;
					case DirType:
						if (filter & tarMode::Dir) 
							addEntryToMap(filename, filesize, type);
						break;
					case FifoType:
						if (filter & tarMode::Fifo)
							addEntryToMap(filename, filesize, type);
						break;
					case ReservedType:
						if (filter & tarMode::Reserved)
							addEntryToMap(filename, filesize, type);
						break;
					default:
						break;
					}
				}

				// Prepare for next header, seek the filesize to the end direction
				_tarfile.seekg(nextEntry, ios_base::cur);

				bytes_read += nextEntry;
				// cout << "entry : " << string(buff) << " filesize : " << filesize << "\n";

			} while (!_tarfile.eof()); // Even if the catch of the NULL header fail, we catch the EOF
			_get_all_entries = true;
		}
		else {
			// cout << "The file closed :( \n";
			_get_all_entries = false;
		}
	}

	bool tarFile::isNullHeader(const char * p)
	{
		int n;
		for (n = 511; n >= 0; --n)
			if (p[n] != '\0')
				return false;
		return true;
	
	}



	/* Parse an octal number, ignoring leading and trailing nonsense. */
	int tarFile::parseoct(const char *p, size_t n)
	{
		int i = 0;

		while ((*p < '0' || *p > '7') && n > 0) {
			++p;
			--n;
		}
		while (*p >= '0' && *p <= '7' && n > 0) {
			i *= 8;
			i += *p - '0';
			++p;
			--n;
		}
		return (i);
	}

	/* Verify the tar checksum. */
	int tarFile::verifyChecksum(const char *p)
	{
		int n, u = 0;
		for (n = 0; n < 512; ++n) {
			if (n < 148 || n > 155)
				/* Standard tar checksum adds unsigned bytes. */
				u += ((unsigned char *)p)[n];
			else
				u += 0x20;

		}
		return (u == parseoct(p + 148, 8));
	}

	map<string, tarEntry *> tarFile::entries;
	ifstream tarFile::_tarfile = ifstream();
}