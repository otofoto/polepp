// POLEPP - Portable C++ library to access OLE Storage 
// Copyright (C) 2004-2006 Jorge Lodos Vigil
// Copyright (C) 2004 Israel Fernandez Cabrera

//   Redistribution and use in source and binary forms, with or without 
//   modification, are permitted provided that the following conditions 
//   are met:
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice, 
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the authors nor the names of its contributors may be 
//     used to endorse or promote products derived from this software without 
//     specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
//   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
//   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
//   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
//   THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#ifndef _OLE_STORAGE_
#define _OLE_STORAGE_

#include "path.hpp"
#include "stream.hpp"

namespace ole
{
	// The compound_document class encapsulates an OLE compound document.
	// This class may iterate all the document's storages. The iterators
	// iterate the current directory (dir_iterator) or the whole document
	// (doc_iterator).
	// Dereferencing an iterator returns an inmutable storage path, which provide 
	// name information and support for several path operations.
	// This class also provides streams for each path that may be used to read
	// and/or modify the compound document.
	// All the functions that receives path names may receive an absolute or 
	// relative path. Absolute paths start with "/". Relative paths must exist
	// in the current directory.
	template<typename _ = void>
	class basic_compound_document
	{
	public:
		// Construction/destruction
		basic_compound_document(std::iostream& ios): m_storage(ios) {}
		basic_compound_document(const std::string& filename, std::ios::openmode mode = std::ios::in, bool create = false);
		~basic_compound_document() { if (m_storage) delete m_storage; }

	// Attributes
	public:
		// Returns true if the document is a valid OLE document.
		bool good() const { return (m_storage && m_storage->result() == POLE::Storage::Ok); }		
		
		// Returns true if the received path exists in the document. Note that
		// path objects always correspond to a valid entry, otherwise the can not
		// be created.
		bool exists(const std::string& name) const { return entry_from_string(name) != NULL; }
		bool exists(const path& p) const { return true; }
		
		// Returns true if the received path is a directory.
		bool is_directory(const std::string& name) const { const POLE::DirEntry* e = entry_from_string(name); return e ? e->dir() : false; }
		bool is_directory(const path& p) const { return p.is_directory(); }
		
		// Returns true if the received path is a file.
		bool is_file(const std::string& name) const { const POLE::DirEntry* e = entry_from_string(name); return e ? e->file() : false; }
		bool is_file(const path& p) const { return p.is_file(); }
		
		// Return the file or directory size as stored in the document. Directory 
		// size should be 0;
		unsigned long entry_size(const std::string& name) const { const POLE::DirEntry* e = entry_from_string(name); return e ? e->size() : 0; }
		unsigned long entry_size(const path& p) const { return p.entry_size(); }
		
		// Return the leaf name of the current directory. For instance, if the current
		// directory is /Macros/VBA returns VBA.
		const std::string& current_dir_relative() const { assert(m_storage); const POLE::DirEntry* e = m_storage->current_entry(); assert(e); return e->name(); }
		
		// Returns the full path of the current directory.
		std::string current_dir_absolute() const { assert(m_storage); const POLE::DirEntry* e = m_storage->current_entry(); assert(e); return absolute_path(e); }

		// Returns the path object for the current directory.
		path current_path() const { assert(m_storage); const POLE::DirEntry* e = m_storage->current_entry(); assert(e); return path(e); }
		
		// Returns the leaf name for any given path object.
		const std::string& relative_path(const path& p) const { return p.relative(); }
		
		// Returns the full path name for any given path object.
		std::string absolute_path(const path& p) const { assert(m_storage); std::string name; m_storage->fullName(p.m_entry, name); return name; }

		// These functions provide a way to have all the paths in the document
		// and the current directory. Must of the time you are better using
		// iterators.
		void entries_in_current_dir(std::vector<path>& paths) const;
		void entries_in_document(std::vector<path>& paths) const;

		// Provide streams used to read/write file content within the storage. If
		// reuse is true only one stream object will be created for the same stream,
		// so you could have several references to the same object. This is safer but
		// slower. It is not advised to have several distinct references to the same
		// file within the document.
		std::auto_ptr<ole::stream> stream(const std::string& name, bool reuse = false) { assert(m_storage); POLE::Stream* s = m_storage->stream(name, reuse); return s ? std::auto_ptr<ole::stream>(new ole::stream(*s)): std::auto_ptr<ole::stream>(); }
		std::auto_ptr<ole::stream> stream(const path& p, bool reuse = false) { assert(m_storage); POLE::Stream* s = m_storage->stream(p.absolute(*this), reuse); return s ? std::auto_ptr<ole::stream>(new ole::stream(*s)): std::auto_ptr<ole::stream>(); }
		
		// Iterator class for iteration over the files in the document. 
		class iterator : public boost::iterator_facade< iterator, path const, boost::single_pass_traversal_tag >
		{
		public:
			iterator(): m_parent(NULL), m_pos(0) {} // Defaults to end() iterator
			iterator(const POLE::Storage* storage, bool dir_iterator);

		private:
			friend class boost::iterator_core_access;

			reference dereference() const { assert(m_paths.size() > m_pos); return m_paths[m_pos]; }
			bool equal( const iterator& rhs ) const { return m_pos == rhs.m_pos && m_parent == rhs.m_parent; }
			void increment();

			const POLE::DirEntry*   m_parent;  // the storage being iterated.
			std::vector<path>       m_paths;   // the children paths.
			std::string::size_type  m_pos;     // position of path in the children array. 
		};
		
		// Current directory iterating functions
		iterator dir_begin() const { return iterator(m_storage, true); }
		iterator dir_end() const { return iterator(); }
		iterator find_in_current_directory(const std::string& path) const;
		
		// Document iterating functions
		iterator doc_begin() const { return iterator(m_storage, false); }
		iterator doc_end() const { return iterator(); }
		iterator find_in_document(const std::string& path) const;

	// Operations
	public:
		// Changes the current directory. Returns true on success.
		bool enter_directory( const std::string& directory ) { assert(m_storage); return m_storage->enterDirectory(directory); }
		
		// Makes the parent directory the current directory. If the current directory is 
		// root it has no effect.
		void leave_directory() { assert(m_storage); return m_storage->leaveDirectory(); }

		// These are not implemented/tested yet.
		bool create_file(const std::string& filename) { assert(m_storage); return m_storage->createFile(filename); }
		bool create_directory(const std::string& directory) { assert(m_storage); return m_storage->createDirectory(directory); }
		bool rename(const std::string& path, const std::string& new_name); 
		bool remove(const std::string& path) { assert(m_storage); return m_storage->delete_entry(path); }
		bool remove(const path& path) { assert(m_storage); return m_storage->delete_entry(path.name()); }

	// Implementation
	private:
		const POLE::DirEntry* entry_from_string(const std::string& name) const { assert(m_storage); return m_storage->getEntry(name); }
		POLE::Storage* m_storage;

		basic_compound_document(); // no default construction
		basic_compound_document(const basic_compound_document<_>&); // no copy construction
		basic_compound_document<_>& operator=(const basic_compound_document<_>&); // no assignment operator
	};

	typedef basic_compound_document<> compound_document;

	////////////////////////////////////////////////////////////////////////////////////////////
	// basic_compound_document implementation
	////////////////////////////////////////////////////////////////////////////////////////////

	template <typename _>
	basic_compound_document<_>::basic_compound_document(const std::string& filename, std::ios::openmode mode, bool create): m_storage(NULL) 
	{
		if (filename.empty())
			return;
		m_storage = new POLE::Storage(filename.c_str(), mode, create);
	}

	template <typename _>
	void basic_compound_document<_>::entries_in_current_dir(std::vector<path>& paths) const
	{
		assert(m_storage);
		
		// get children
		std::vector<const POLE::DirEntry*> entries;
		m_storage->listEntries(entries);
		
		// Initialize paths
		std::vector<const POLE::DirEntry*>::iterator it;
		for (it = entries.begin(); it != entries.end(); ++it)
			paths.push_back(path(*it)); // pointers will remain valid as long as the storage
	}

	template <typename _>
	void basic_compound_document<_>::entries_in_document(std::vector<path>& paths) const
	{
		assert(m_storage);
		
		// get children
		std::vector<const POLE::DirEntry*> entries;
		m_storage->listAll(entries);
		
		// Initialize paths
		std::vector<const POLE::DirEntry*>::iterator it;
		for (it = entries.begin(); it != entries.end(); ++it)
			paths.push_back(path(*it)); // pointers will remain valid as long as the storage
	}

	template<class _>
	typename basic_compound_document<_>::iterator basic_compound_document<_>::find_in_current_directory(const std::string& path) const
	{
		iterator it = dir_begin();
		for (; it != dir_end(); ++it)
			if (it->leaf() == path)
				break;
		return it;
	}

	template<class _>
	typename basic_compound_document<_>::iterator basic_compound_document<_>::find_in_document(const std::string& path) const
	{
		const POLE::DirEntry* e = entry_from_string(path);
		if (!e)
			return doc_end();

		iterator it = doc_begin();
		for (; it != doc_end(); ++it)
			if (it->absolute(*this) == path)
				break;
		return it;
	}

	template<class _>
	basic_compound_document<_>::iterator::iterator(const POLE::Storage* storage, bool dir_iterator): m_pos(0)
	{
		assert(storage);
		
		// get children
		std::vector<const POLE::DirEntry*> entries;
		if (dir_iterator)
			storage->listEntries(entries);
		else
			storage->listAll(entries);
		
		// Initialize paths
		std::vector<const POLE::DirEntry*>::iterator it;
		for (it = entries.begin(); it != entries.end(); ++it)
			m_paths.push_back(path(*it)); // pointers will remain valid as long as the storage
		
		// get parent
		m_parent = dir_iterator ? storage->current_entry() : storage->root_entry();
		if (m_paths.empty())
			m_parent = NULL; // Set to end() iterator
	} 

	template<class _>
	void basic_compound_document<_>::iterator::increment()
	{
		assert( m_pos < m_paths.size() );
		m_pos++;
		if (m_pos == m_paths.size())
		{
			// Set to end()
			m_parent = NULL;
			m_paths.clear();
			m_pos = 0;
		}
	}

}

#endif // _OLE_STORAGE_
