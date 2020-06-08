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
#ifndef _OLE_STREAM_
#define _OLE_STREAM_

#include "pole/pole.h"

namespace ole
{
	// Objects of this class are constructed by the storage. 
	// Streams allow read/write operations. Allthough not typical for
	// streams, they also have a size and may be resized. This
	// seemed better than automatically resize on overflow.
	// These streams are dual seekable, they have separate heads
	// for reading and writing.
	// This class is not thread safe.
	class stream
	{
	// Construction
	public:	
		stream(POLE::Stream& str): m_stream(str) {}
		stream(const stream& other): m_stream(other.m_stream) {}

	// Attributes
	public:
		// Standard stream attributes.
		bool fail() const { return m_stream.fail(); }
		bool eof() const { return m_stream.eof(); }
		std::streampos tellg() const { return m_stream.tellg(); }
		std::streampos tellp() const { return m_stream.tellp(); }
		std::streamsize size() const { return m_stream.size(); }
	
	// Operations
	public:
		// These are standard stream operations. This functions may set or 
		// reset the eof and fail bits.
		std::streamsize read(char* buf, std::streamsize n) { return m_stream.read((unsigned char *)buf, n); }
		std::streamsize write(const char* buf, std::streamsize n) { return m_stream.write((unsigned char *)buf, n); }
		std::streampos seek(std::streamoff off, std::ios::seekdir way, std::ios::openmode mode) { return m_stream.seek(off, way, mode); }
		void seekg(std::streamoff off, std::ios::seekdir way) { m_stream.seekg(off, way); }
		void seekp(std::streamoff off, std::ios::seekdir way) { m_stream.seekp(off, way); }
		
		// Increase the stream size. If size is less than the current size no
		// action is taken and the function succeeds. The value of new space
		// is undefined. Returns true on success.
		bool reserve(std::streamsize size) { return m_stream.reserve(size); }
		
		// Set a new size for the stream, making it smaller if size is less
		// than the current size. When increasing the size, the value of new
		// space is filled with val. Returns true on success.
		bool resize(std::streamsize size, char val = 0) { return m_stream.resize(size, val); }

	// Implementation
	private:
		POLE::Stream& m_stream;

		stream(); // No default construction
		stream& operator=( const stream& other ); // No asignment operator 
	};
}

#endif // _OLE_STREAM_
