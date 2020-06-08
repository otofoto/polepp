// POLEPP - Portable C++ library to access OLE Storage 
// Copyright (C) 2004-2006 Jorge Lodos Vigil
// Copyright (C) 2004 Israel Fernandez Cabrera

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include "../includes/polepp.hpp"

// Print directory contents
void dir( const ole::compound_document& doc )
{
	ole::compound_document::iterator it = doc.doc_begin();
	for (; it != doc.doc_end(); ++it)
	{
		assert(doc.exists(it->absolute(doc)));
		if (it->is_directory())
		{
			std::cout << "Directory: " << it->absolute(doc).c_str() << std::endl;
		}
		else
		{
			assert(doc.is_file(it->absolute(doc)));
			std::cout << "File entry: " << it->absolute(doc) << "    size is: " << doc.entry_size(*it) << std::endl;
		}
	}
}

// Recursively print directory contents. The parameter is not const
// because entering and leaving directories modify the object.
void dir_recursive( ole::compound_document& doc )
{
	ole::compound_document::iterator it = doc.dir_begin();
	for (; it != doc.dir_end(); ++it)
	{
		assert(doc.exists(it->absolute(doc)));
		if (it->is_directory())
		{
			std::cout << "Entering directory: " << it->absolute(doc) << std::endl;
			bool res = doc.enter_directory(it->absolute(doc));
			std::cout << "Current directory is: " << doc.current_dir_absolute().c_str() << std::endl;
			assert(res);
			dir_recursive(doc);
			std::cout << "Leaving directory: " << it->absolute(doc) << std::endl;
			doc.leave_directory();
			std::cout << "Current directory is: " << doc.current_dir_absolute().c_str() << std::endl;
		}
		else
		{
			assert(doc.is_file(it->absolute(doc)));
			std::cout << it->absolute(doc) << "    size is: " << doc.entry_size(*it) << std::endl;
		}
	}
}

void create_dir(const ole::compound_document& doc, const ole::path& path, const boost::filesystem::path& root)
{
	boost::filesystem::path new_path = root;
	ole::path::iterator it;
	for (it = path.begin(doc); it != path.end(); ++it)
	{
		if (*it == "/")
			continue;
		new_path /= *it;
		if (!boost::filesystem::exists(new_path))
			boost::filesystem::create_directory(new_path);
	}
}

// Extract directory contents. The parameter is not const
// because reading from streams modify the object (stream
// current read position).
bool extract(ole::compound_document& doc, const boost::filesystem::path& folder)
{
	// Create directories first
	ole::compound_document::iterator it;
	for (it = doc.doc_begin(); it != doc.doc_end(); ++it)
	{
		assert(doc.exists(it->absolute(doc)));
		if (it->is_directory())
		{
			boost::filesystem::path new_path = folder;
			new_path /= it->absolute(doc);
			std::cout << "Creating directory: " << new_path.string().c_str() << std::endl;
			create_dir(doc, *it, folder);
		}
	}

	// Save the streams
	for (it = doc.doc_begin(); it != doc.doc_end(); ++it)
	{
		assert(doc.exists(it->absolute(doc)));
		if (it->is_file())
		{
			assert(doc.is_file(it->absolute(doc)));
			std::auto_ptr<ole::stream> s = doc.stream(*it);
			boost::filesystem::path new_path = folder;
			new_path /= it->branch(doc);
			std::string filename = it->leaf();
			if (filename[0] < ' ') // Make sure the filename is valid
				filename.erase(0, 1);
			new_path /= filename;
			std::cout << "Saving file: " << new_path.string().c_str() << std::endl;
			std::streamsize size = s->size();
			// In a real world program this would be a fixed size buffer used within a
			// read/write loop. Checking on read result is also needed.
			char* buf = new char[size];
			s->read(buf, size);
			std::ofstream os(new_path.string().c_str(), std::ios::binary);
			if (os.fail())
				return false;
			os.write(buf, size);
			delete[] buf;
		}
	}

	return true;
}

int die(const std::string& msg)
{
	std::cout << msg << std::endl;
	exit(-1);
}


int main(int argc, char* argv[])
{
	// A file name and destination directory must be supplied
	if (argc != 3)
	{
		die("Usage: pole_pp_test <file_name> <folder_name>");
	}

	try // catch filesystem errors
	{
		// Verify input file
		boost::filesystem::path file(argv[1], boost::filesystem::native);
		if (!boost::filesystem::exists(file))
			die("File not found.");

		// Create a compound_document.
		ole::compound_document doc(argv[1]);

		// Must always check validity.
		if (!doc.good())
		{
			std::cout << "Fatal error, probable causes: " << std::endl;
			std::cout << "1. The given file is not an OLE file" << std::endl;
			std::cout << "2. The given file is an OLE file, but its internal structure is corrupted" << std::endl;
			std::cout << "If you think this is a pole++ error, please send the file to the library authors." << std::endl;
			std::cout << std::endl << "done..." << std::endl;
			return -2;
		}

		// Recursively print directory contents
		dir_recursive( doc );
		std::cout << std::endl << std::endl;

		// Print directory contents
		dir( doc );
		std::cout << std::endl << std::endl;

		// Check the find functions
		std::cout << "Current directory is: " << doc.current_dir_absolute().c_str() << std::endl;
		ole::compound_document::iterator it = doc.find_in_current_directory("Macros");
		if (it != doc.dir_end())
			std::cout << "Path Macros found in the current directory." << std::endl;
		else
			std::cout << "Path Macros not found in the current directory." << std::endl;
		it = doc.find_in_document("/Macros/VBA");
		if (it != doc.dir_end())
			std::cout << "Path /Macros/VBA found in the document." << std::endl;
		else
			std::cout << "Path /Macros/VBA not found in the document." << std::endl;
		std::cout << std::endl;

		// Path iterator functions. Try to enter the /Macros/VBA storage, which may not exist
		bool res = doc.enter_directory("/Macros/VBA");
		ole::path cur_path = doc.current_path();
		ole::path::iterator path_it;
		for (path_it = cur_path.begin(doc); path_it != cur_path.end(); ++path_it)
			std::cout << "Path element: " << path_it->c_str() << std::endl;
		std::cout << std::endl;
		
		// Create destination folder if needed
		boost::filesystem::path folder(argv[2], boost::filesystem::native);
		if (!boost::filesystem::exists(folder))
			boost::filesystem::create_directory(folder);

		// Extract all streams and save to disk
		res = extract(doc, folder);
		assert(res);
	}
	catch(boost::filesystem::filesystem_error e)
	{
		std::cout << "Error!" << std::endl;
		std::cout << e.what() << std::endl;
	}

	std::cout << "Done." << std::endl;
	return 0;
}

