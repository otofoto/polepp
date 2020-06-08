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


// Some code below was borrowed and modified from path.hpp and operations_posix_windows.cpp
// in the boost::filesystem library version 1.33.1 with the following copyright:

// Copyright © 2002 Beman Dawes
// Copyright © 2001 Dietmar Kühl
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef _OLE_PATH_
#define _OLE_PATH_

#include <boost/iterator/iterator_facade.hpp>
#include "pole/pole.h"


namespace ole
{
	template<typename _>
	class basic_compound_document; // Forward declaration
	
	// Objects of this class are inmutable, they are constructed by the storage. 
	// Its only purpose is to provide information about a specific path.
	// The interface of this class is modelled after boost::filesystem::path,
	// some code and ideas were borrowed from boost::filesystem::path sources.
	// This class is not thread safe.
	template<class _ = void>
	class basic_path
	{
	// Construction
	public:
		basic_path(const POLE::DirEntry* entry): m_entry(entry) {}
		// Copy construction and operator= are needed to have std::vector<path>
		basic_path(const basic_path<_>& rhs): m_entry(rhs.m_entry) {}
		basic_path<_>& operator=(const basic_path<_>& rhs) { m_entry = rhs.m_entry; return *this; }
	    
	// Attributes
	public:
		// Returns the full path.
		std::string absolute(const basic_compound_document<void>& doc) const { assert(m_entry); return doc.absolute_path(m_entry); }
		
		// Return the size for this path as stored in the document.
		unsigned long entry_size() const { assert(m_entry); return m_entry->size(); }

		// Returns true if this is the root entry
		bool is_root() const { assert(m_entry); return m_entry->root(); }

		// Returns true if this is a directory
		bool is_directory() const { assert(m_entry); return m_entry->dir(); }
		
		// Returns true if this is a file
		bool is_file() const { assert(m_entry); return m_entry->file(); }

		// Returns true if the entry name is empty.
		bool unnamed() const { assert(m_entry); return m_entry->name().empty(); }
		
		// Returns the parent directory
		std::string branch(const basic_compound_document<void>& doc) const;
		
		// Returns the entry relative name.For instance, if this is "/Macro/VBA"
		// returns "VBA"
		std::string leaf() const { assert(m_entry); return m_entry->name(); }

		// relational operators
		bool operator<( const basic_path<_>& rhs ) const { return std::lexicographical_compare( begin(), end(), rhs.begin(), rhs.end() ); }
		bool operator==( const basic_path<_>& rhs ) const { return !(*this < rhs) && !(rhs < *this); }
		bool operator!=( const basic_path<_>& rhs ) const { return !(*this == rhs); }
		bool operator>( const basic_path<_>& rhs ) const  { return rhs < *this; }
		bool operator<=( const basic_path<_>& rhs ) const { return !(rhs < *this); }
		bool operator>=( const basic_path<_>& rhs ) const { return !(*this < rhs); }

		// iteration over the names in the path
		class iterator : public boost::iterator_facade< iterator, std::string const, boost::bidirectional_traversal_tag >
		{
		public:
			iterator(): m_pos(0) {} 
			iterator(const std::string& full_path): m_full_path(full_path), m_name(first_name(full_path)), m_pos(0) {} 

		private:
			friend class boost::iterator_core_access;

			reference dereference() const { assert(m_pos < m_full_path.length()); return m_name; }
			bool equal( const iterator& rhs ) const;
			void increment();
			void decrement();

			static std::string first_name( const std::string& src );
			std::string             m_name;      // cache current element.
			std::string             m_full_path; // path being iterated over.
			std::string::size_type  m_pos;       // position of name in path_ptr->string(). The end() 
												 // iterator is indicated by pos == m_full_path.size()
		};

		iterator begin(const basic_compound_document<void>& doc) const { assert(m_entry); return iterator(doc.absolute_path(*this)); }
		iterator end() const { return iterator(); }
	  
	// Implementation
	private:
		friend class basic_compound_document<void>;
		
		static std::string::size_type leaf_pos( const std::string& str, std::string::size_type end_pos ); // end_pos is past-the-end position
		const POLE::DirEntry* m_entry;
		
		basic_path(); // no default construction
	};

	typedef basic_path<> path;

	////////////////////////////////////////////////////////////////////////////////////////////
	// basic_path implementation
	////////////////////////////////////////////////////////////////////////////////////////////

	template<class _>
	std::string basic_path<_>::branch(const basic_compound_document<void>& doc) const
	{
		assert(m_entry);
		std::string absolute = doc.absolute_path(*this);
		std::string::size_type end_pos( leaf_pos( absolute, absolute.size() ) );

		// skip a '/' unless it is a root directory
		if ( end_pos && absolute[end_pos-1] == '/' && end_pos != 1 )
			--end_pos;
		return absolute.substr( 0, end_pos );
	}

	template<class _>
	std::string::size_type basic_path<_>::leaf_pos( const std::string& str, std::string::size_type end_pos ) // end_pos is past-the-end position
	{
		// return 0 if str itself is leaf (or empty) 
		if ( end_pos && str[end_pos-1] == '/' )
			return end_pos-1;

		std::string::size_type pos( str.find_last_of( '/', end_pos-1 ) );
		if (pos == std::string::npos)
			return 0; // path itself must be a leaf (or empty) so leaf is entire string or starts after delimiter
		return pos + 1;
	}
		
	template<class _>
	bool basic_path<_>::iterator::equal( const iterator& rhs ) const
	{
		return 
			(m_full_path == rhs.m_full_path && m_pos == rhs.m_pos) || 
			// for end() iterator
			(m_pos == m_full_path.size() && rhs.m_pos == rhs.m_full_path.size());
	}

	template<class _>
	void basic_path<_>::iterator::increment()
	{
		assert( m_pos < m_full_path.length() ); // detect increment past end
		m_pos += m_name.size();
		if ( m_pos == m_full_path.length() )
		{
			m_name = "";  // not strictly required, but might aid debugging
			return;
		}
		if ( m_full_path[m_pos] == '/' )
			++m_pos;
		std::string::size_type end_pos( m_full_path.find( '/', m_pos ) );
		if ( end_pos == std::string::npos )
			end_pos = m_full_path.length();
		m_name = m_full_path.substr( m_pos, end_pos - m_pos );
	}

	template<class _>
	void basic_path<_>::iterator::decrement()
	{                                                                                
		assert( m_pos ); // detect decrement of begin
		std::string::size_type end_pos( m_pos );

		// skip a '/' unless it is the root directory
		if ( m_full_path[end_pos-1] == '/' && end_pos != 1 )
			--end_pos;
		m_pos = basic_path<_>::leaf_pos( m_full_path, end_pos );
		m_name = m_full_path.substr( m_pos, end_pos - m_pos );
	}

	template<class _>
	std::string basic_path<_>::iterator::first_name( const std::string& src )
	{
		std::string target;
		std::string::const_iterator itr( src.begin() );
		while ( itr != src.end() && *itr != '/' )
			target += *itr++;
		if ( itr != src.end() )
		{
			// *itr is '/'
			if ( itr == src.begin() )
				target += '/';
		}
		return target;
	}

} // namespace ole

#endif // _OLE_PATH_
