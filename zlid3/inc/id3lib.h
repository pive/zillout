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
#define ZL_ERR_INVALID_FILE 0x00000005

#define ZL_INVALID_UID      ((uint32_t)0xFFFFFFFF)

/* libid3tag API undeclared functions
-------------------------------------------------------------------------------
*/
# ifdef __cplusplus
extern "C" {
# endif

id3_length_t id3_ucs4_utf8size(id3_ucs4_t const *);
id3_length_t id3_utf8_length(id3_utf8_t const *);
id3_length_t id3_utf8_size(id3_utf8_t const *);
void         id3_utf8_encode(id3_utf8_t *, id3_ucs4_t const *);

# ifdef __cplusplus
}
# endif

/* Base object
-------------------------------------------------------------------------------
*/
class ZLID3Object
{
private:
	uint32_t _uid;

protected:
	ZLID3Object() { this->_uid = ZL_INVALID_UID; };

public:
	void* operator new (size_t size, ZLFastHeap* pHeap) { return pHeap->alloc(size); };
	void  operator delete (void * p) {};

	void uid(uint32_t uid)   { this->_uid = uid; };
	uint32_t uid()           { return this->_uid; };
};

/* Image object
-------------------------------------------------------------------------------
*/
class ZLID3Image
{
private:
	char*  pImageMimeType;
	char*  pszSourceFilePath;
	int    picType;
	size_t sImageSize;
	void*  pImageBuffer;

public:
	ZLID3Image(bool, const char*, id3_frame*);
	ZLID3Image(ZLID3Image*);
	ZLID3Image(unsigned char*, size_t, const char*);
	~ZLID3Image();

	void*  buffer();
	size_t size()   { return this->sImageSize; };
	char*  mime()   { return this->pImageMimeType; };
	int    type()   { return this->picType; };
	char*  source() { return this->pszSourceFilePath; };
};

/* Track object
-------------------------------------------------------------------------------
*/
class ZLID3Track: public ZLID3Object
{
private:
	ZLFastHeap* pHeap;
	const char* pszFilePath;
	id3_frame*  pImageFrame;

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

/* EventHandler object
-------------------------------------------------------------------------------
*/
class ZLID3EventHandler
{
public:
	virtual void onLoadFile(const char* pszError, const char* pszPath, ZLID3Track* pTrack, uint32_t uProcessed) = 0;
	virtual void onComplete(const char* pszError, class ZLID3Library* pLibrary) = 0;
};

/* Album object
-------------------------------------------------------------------------------
*/
class ZLID3Album: public ZLID3Object
{
private:
	id3_utf8_t* pName;
	id3_utf8_t* pArtist;
	ZLID3Image* pImage;

	ZLArrayList<ZLID3Track> _tracks;
public:
	ZLID3Album(id3_utf8_t*, id3_utf8_t*);
	~ZLID3Album();

	void addTrack(ZLID3Track* pTrack);
	void setImage(bool, const char*, id3_frame*);
	void setImage(ZLID3Image*);

	id3_utf8_t* name()                { return this->pName; };
	id3_utf8_t* artist()              { return this->pArtist; };
	ZLID3Image* image()               { return this->pImage; };
	ZLArrayList<ZLID3Track>* tracks() { return &this->_tracks; }

	static id3_utf8_t* index(ZLFastHeap*, id3_utf8_t*, id3_utf8_t*);
};

/* Artist object
-------------------------------------------------------------------------------
*/
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

/* Library object
-------------------------------------------------------------------------------
*/
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

		ZLArrayList<ZLID3Track> _tracks;

		ZLFastDict* pArtistDict;
		ZLFastDict* pAlbumDict;

		const id3_utf8_t* pDefaultAlbum;
		const id3_utf8_t* pDefaultArtist;

		ZLID3Artist* pUnknownArtist;

		static ZLID3Image* pDefaultImage;

		static int   _readFileID3Tag(void* pData, const char* pszBase, const char* pszFile);
		static int   _countFiles(void* pData, const char* pszBase, const char* pszFile);
		static void* _pathHeapAlloc(void* pData, size_t desired);
		static void* _disposableHeapAlloc(void* pData, size_t desired);

	public:
		ZLID3Library(const char* pszBasePath);
		~ZLID3Library();

		void loadImages(bool fLoadImages) { this->fLoadImages = fLoadImages; };
		void load(ZLID3EventHandler* pHandler);

		ZLID3Image* getAlbumImage(id3_utf8_t* pArtistName, id3_utf8_t* pAlbumName);
		ZLID3Image* getTrackImage(uint32_t);
		ZLID3Track* getTrack(uint32_t);

		ZLArrayList<ZLID3Track>* getTrackList() { return &this->_tracks; };

		void dump();
		ZLFastHeap* heap() { return this->pPathHeap; }
		const char* path() { return this->pszBasePath; }

		static ZLID3Image* getDefaultImage();
};
