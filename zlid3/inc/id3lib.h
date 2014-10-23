/*
 ------------------------------------------------------------------------------
	zillout - a simple audio library manager for node.js

	Copyright (C) 2014 Pierre Veber
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
------------------------------------------------------------------------------
*/


/* Quick mp3 library management C++ API (wrapper around libid3tag API)
-------------------------------------------------------------------------------
*/

struct _ID3Info_t;
struct _ID3Image_t;

#define ZL_ERR_ALLOCATE     0x00000001
#define ZL_ERR_GET_FIELD    0x00000002
#define ZL_ERR_OPEN_FD      0x00000003
#define ZL_ERR_READ_TAG     0x00000004

class ZLID3Object
{
public:
	void* operator new (size_t size, ZLFastHeap* pHeap) { return pHeap->alloc(size); };
	void  operator delete (void * p) {};
};

class ZLID3Image
{
private:
	const id3_latin1_t* pImageMimeType;
	const char* pszSourceFilePath;

	int    picType;
	size_t sImageSize;
	void*  pImageBuffer;

public:
	ZLID3Image(bool, const char*, id3_frame*);

	void*  buffer();
	size_t size() { return this->sImageSize; };
	char*  mime() { return (char*)this->pImageMimeType; };
};

class ZLID3Track: public ZLID3Object
{
private:
	ZLFastHeap* pHeap;
	const char* pszFilePath;
	id3_frame* pImageFrame;

	id3_utf8_t* pArtist;
	id3_utf8_t* pAlbum;
	id3_utf8_t* pTitle;
	id3_utf8_t* pTrackNum;
	id3_utf8_t* pTrackLen;
	id3_utf8_t* pYear;

	id3_utf8_t* getUTF8Value(id3_frame* pID3Frame);

public:
	ZLID3Track(ZLFastHeap*, const char*, id3_tag*);

	id3_frame*  imageFrame() { return this->pImageFrame; };
	id3_utf8_t* artist()     { return this->pArtist; };
	id3_utf8_t* album()      { return this->pAlbum; };
	id3_utf8_t* title()      { return this->pTitle; };
	id3_utf8_t* tracknum()   { return this->pTrackNum; };
	id3_utf8_t* tracklen()   { return this->pTrackLen; };
	id3_utf8_t* year()       { return this->pYear; };
	const char* path()       { return this->pszFilePath; };
};

class ZLID3EventHandler
{
public:
	virtual void onLoadFile(const char* pszError, const char* pszPath, ZLID3Track* pTrack, uint32_t uProcessed) = 0;
	virtual void onComplete(const char* pszError, class ZLID3Library* pLibrary) = 0;
};

class ZLID3Album: public ZLID3Object
{
private:
	id3_utf8_t* pName;
	id3_utf8_t* pArtist;
	ZLID3Image* pImage;

	ZLArrayList<ZLID3Track> tracks;
public:
	ZLID3Album(id3_utf8_t*, id3_utf8_t*);
	void addTrack(ZLID3Track* pTrack);
	void setImage(bool, const char*, id3_frame*);

	id3_utf8_t* name()       { return this->pName; };
	id3_utf8_t* artist()     { return this->pArtist; };
	ZLID3Image* image()      { return this->pImage; };

	static id3_utf8_t* index(ZLFastHeap*, id3_utf8_t*, id3_utf8_t*);
};

class ZLID3Artist: public ZLID3Object
{
private:
	const id3_utf8_t* pName;

	ZLArrayList<ZLID3Album> albums;
	ZLArrayList<ZLID3Track> tracks;
public:
	ZLID3Artist(const id3_utf8_t*);
	const id3_utf8_t* name() { return this->pName; };
	void addAlbum(ZLID3Album* pAlbum);
	void addTrack(ZLID3Track* pTrack);
};

class ZLID3Library
{
	private:
		ZLFastHeap* pPathHeap;
		ZLFastHeap* pTrackHeap;
		ZLFastHeap* pDisposableHeap;

		ZLID3EventHandler* pHandler;

		char* pszBasePath;
		int   err;
		bool  fLoadImages;

		uint32_t uTotalFiles;
		uint32_t uInvalidFiles;
		uint32_t uProcessedFiles;

		uint32_t uAlbums;
		uint32_t uArtists;
		uint32_t uTracks;
		uint32_t uImages;

		uint32_t uAllocated;

		ZLFastDict* pArtistDict;
		ZLFastDict* pAlbumDict;

		const id3_utf8_t* pDefaultAlbum;
		const id3_utf8_t* pDefaultArtist;

		ZLID3Artist* pUnknownArtist;

		static int   _readFileID3Tag(void* pData, const char* pszBase, const char* pszFile);
		static int   _countFiles(void* pData, const char* pszBase, const char* pszFile);
		static void* _pathHeapAlloc(void* pData, size_t desired);
		static void* _disposableHeapAlloc(void* pData, size_t desired);

	public:
		ZLID3Library(const char* pszBasePath);
		~ZLID3Library();

		void loadImages(bool fLoadImages) { this->fLoadImages = fLoadImages; };
		void load(ZLID3EventHandler* pHandler);

		void dump();
		void dumpImage(const char* pszAlbum, const char* pszOutFile);
		ZLFastHeap* heap() { return this->pPathHeap; }
};
