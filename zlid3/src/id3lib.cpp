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

#include "../inc/precomp.h"


#define PATH_HEAP_SIZE     2048
#define TRACK_HEAP_SIZE    2048
#define DISP_HEAP_SIZE     2048

#define ARTIST_DICT_SIZE   1024
#define ALBUM_DICT_SIZE    1024


/* ZLID3Track base
-------------------------------------------------------------------------------
*/

ZLID3Track::ZLID3Track(ZLFastHeap* pHeap, const char* pszPath, id3_tag* pID3Tag)
{
	this->pArtist   = NULL;
	this->pAlbum    = NULL;
	this->pTitle    = NULL;
	this->pTrackNum = NULL;
	this->pTrackLen = NULL;
	this->pYear     = NULL;

	this->pImageFrame = NULL;
	this->pszFilePath = pszPath;
	this->pHeap       = pHeap;

	id3_frame* pID3Frame;
	for (int i=0; i < (int)pID3Tag->nframes; i++)
	{
		pID3Frame = pID3Tag->frames[i];
		if (strcmp("TIT2", pID3Frame->id) == 0)
		{
			/* track title */
			this->pTitle = this->getUTF8Value(pID3Frame);
			continue;
		}
		else if (strcmp("TALB", pID3Frame->id) == 0)
		{
			/* album title */
			this->pAlbum = this->getUTF8Value(pID3Frame);
			continue;
		}
		else if (strcmp("TRCK", pID3Frame->id) == 0)
		{
			/* track number */
			this->pTrackNum = this->getUTF8Value(pID3Frame);
			continue;
		}
		else if (strcmp("TPE1", pID3Frame->id) == 0)
		{
			/* artist name */
			this->pArtist= this->getUTF8Value(pID3Frame);
			continue;
		}
		else if (strcmp("TLEN", pID3Frame->id) == 0)
		{
			/* length of track in ms */
			this->pTrackLen = this->getUTF8Value(pID3Frame);
			continue;
		}
		else if (	(strcmp("TDRC", pID3Frame->id) == 0)
				 || (strcmp("TYER", pID3Frame->id) == 0)	)
		{
			/* release date */
			this->pYear = this->getUTF8Value(pID3Frame);
			continue;
		}
		else if (	(strcmp("APIC", pID3Frame->id) == 0)
				 && pID3Frame->nfields == 5 )
		{
			this->pImageFrame = pID3Frame;
			continue;
		}
		else if (strcmp("ZOBS", pID3Frame->id) == 0 && pID3Frame->nfields == 2)
		{
			id3_field* pFrameId = id3_frame_field(pID3Frame, 0);
			id3_field* pBinary  = id3_frame_field(pID3Frame, 1);

			if (    id3_field_type(pFrameId) == ID3_FIELD_TYPE_FRAMEID
				 && id3_field_type(pBinary) == ID3_FIELD_TYPE_BINARYDATA    )
			{
				const char* obsid = id3_field_getframeid(pFrameId);
				if (strcmp(obsid, "TYER") == 0)
				{
					id3_length_t datasize;
					const id3_byte_t* data = id3_field_getbinarydata(pBinary, &datasize);

					this->pYear = (id3_utf8_t*)this->pHeap->alloc(datasize+1);
					if (data[0] == '\0')
					{
						datasize--;
						memcpy(this->pYear, data+1, datasize);
					}
					else
					{
						memcpy(this->pYear, data, datasize);
					}
					this->pYear[datasize] = '\0';
					continue;
				}
			}
		}
		//fprintf(stderr, "ZLID3Track::ZLID3Track(): unused frame id: %s\n", pID3Frame->id);
	}
}

id3_utf8_t* ZLID3Track::getUTF8Value(id3_frame* pID3Frame)
{
	const id3_ucs4_t* pUCS4 = NULL;
	id3_utf8_t* pValue = NULL;
	id3_length_t nSize = 0;

	for (unsigned int ui=0; (ui < pID3Frame->nfields) && (pUCS4 == NULL); ui++)
	{
		id3_field* pID3Field = id3_frame_field(pID3Frame, ui);
		switch (id3_field_type(pID3Field))
		{
			case ID3_FIELD_TYPE_STRINGLIST:
				pUCS4 = id3_field_getstrings(pID3Field, 0);
				break;

			case ID3_FIELD_TYPE_STRING:
				pUCS4 = id3_field_getstring(pID3Field);
				break;

			default:
				break;
		}
	}

	if (pUCS4 == NULL)
	{
		return NULL;
	}

	nSize  = id3_ucs4_utf8size(pUCS4);
	pValue = (id3_utf8_t*)this->pHeap->alloc(nSize*sizeof(id3_utf8_t));
	if (pValue == NULL)
	{
		return NULL;
	}
		
	id3_utf8_encode(pValue, pUCS4);

	return pValue;
}

ZLID3Image::ZLID3Image(bool fLoad, const char* pszSource, id3_frame* pID3Frame)
{
	this->pImageBuffer      = NULL;
	this->sImageSize        = 0;
	this->pszSourceFilePath = _strdup(pszSource);
	this->pImageMimeType    = _strdup((char*)id3_field_getlatin1(id3_frame_field(pID3Frame, 1)));
	this->picType           = (int)id3_field_getint(id3_frame_field(pID3Frame, 2));

	id3_field* pBinary = id3_frame_field(pID3Frame, 4);
	if (pBinary != NULL && pBinary->type == ID3_FIELD_TYPE_BINARYDATA)
	{
		this->sImageSize = pBinary->binary.length;
		if (fLoad)
		{
			this->pImageBuffer = malloc(pBinary->binary.length);
			if (this->pImageBuffer != NULL)
			{
				memcpy(this->pImageBuffer, pBinary->binary.data, pBinary->binary.length);
			}
		}
	}
}

ZLID3Image::ZLID3Image(ZLID3Image* pCopy)
{
	this->sImageSize        = pCopy->size();
	this->pszSourceFilePath = _strdup(pCopy->source());
	this->pImageMimeType    = _strdup(pCopy->mime());
	this->picType           = pCopy->type();

	if (pCopy->buffer() != NULL)
	{
		this->pImageBuffer = malloc(this->sImageSize);
		if (this->pImageBuffer != NULL)
		{
			memcpy(this->pImageBuffer, pCopy->buffer(), this->sImageSize);
		}
	}
}

ZLID3Image::~ZLID3Image()
{
	free(this->pszSourceFilePath);
	free(this->pImageMimeType);
	if (this->pImageBuffer != NULL)
	{
		free(this->pImageBuffer);
	}
}

void* ZLID3Image::buffer()
{
	if (this->pImageBuffer == NULL && this->sImageSize != 0)
	{
	 	id3_file*  pID3File;

		/* get ID3 tags */
		pID3File = id3_file_open(this->pszSourceFilePath, ID3_FILE_MODE_READONLY);
		if (pID3File != 0)
		{
			id3_tag* pID3Tag = id3_file_tag(pID3File);
			id3_frame* pID3Frame;
			for (int i=0; i < (int)pID3Tag->nframes; i++)
			{
				pID3Frame = pID3Tag->frames[i];
				if (    (strcmp("APIC", pID3Frame->id) == 0)
				     && pID3Frame->nfields == 5 )
				{
					id3_field* pBinary = id3_frame_field(pID3Frame, 4);
					if (pBinary != NULL && pBinary->type == ID3_FIELD_TYPE_BINARYDATA)
					{
						this->sImageSize = pBinary->binary.length;
						this->pImageBuffer = malloc(pBinary->binary.length);
						if (this->pImageBuffer != NULL)
						{
							memcpy(this->pImageBuffer, pBinary->binary.data, pBinary->binary.length);
						}
					}
				}
			}
		}
	}
	return this->pImageBuffer;
}


ZLID3Album::ZLID3Album(id3_utf8_t* pName, id3_utf8_t* pArtist)
{
	this->pArtist = pArtist;
	this->pName   = pName;
	this->pImage  = NULL;

	this->_tracks = ZLArrayList<ZLID3Track>();
}

ZLID3Album::~ZLID3Album()
{
	if (this->pImage != NULL)
	{
		delete this->pImage;
	}
}

id3_utf8_t* ZLID3Album::index(ZLFastHeap* pHeap, id3_utf8_t* pName, id3_utf8_t* pArtist)
{
	id3_length_t sArtist = id3_utf8_size(pArtist);
	id3_length_t sAlbum  = id3_utf8_size(pName);
    id3_utf8_t   pSepr   = (id3_utf8_t)0x3A;
	id3_utf8_t*  pIndex;

	if (pHeap != NULL)
	{
		pIndex = (id3_utf8_t*)pHeap->alloc(sArtist + sAlbum + 1);
	}
	else
	{
		pIndex = (id3_utf8_t*)malloc(sArtist + sAlbum + 1);
	}

	id3_utf8_t* pValue = pIndex;
	memcpy(pValue, pArtist, sArtist);
	pValue += id3_utf8_length(pArtist);
	memcpy(pValue, &pSepr, 1);
	pValue += 1;
	memcpy(pValue, pName, sAlbum);

	return pIndex;
}

void ZLID3Album::addTrack(ZLID3Track* pTrack)
{
	this->_tracks.push(pTrack);
}

void ZLID3Album::setImage(bool fLoad, const char* pszSource, id3_frame* pID3Frame)
{
	if (this->pImage == NULL)
	{
		this->pImage = new ZLID3Image(fLoad, pszSource, pID3Frame);
	}
}

void ZLID3Album::setImage(ZLID3Image* pImage)
{
	this->pImage = new ZLID3Image(pImage);
}

ZLID3Artist::ZLID3Artist(const id3_utf8_t* pName)
{
	this->pName   = pName;

	this->albums  = ZLArrayList<ZLID3Album>();
	this->tracks  = ZLArrayList<ZLID3Track>();
}

void ZLID3Artist::addAlbum(ZLID3Album* pAlbum)
{
	this->albums.push(pAlbum);
}

void ZLID3Artist::addTrack(ZLID3Track* pTrack)
{
	this->tracks.push(pTrack);
}

/* Quick mp3 tag management C++ API (wrapper around libid3tag API)
-------------------------------------------------------------------------------
*/

ZLID3Library::ZLID3Library(const char* pszFilePath)
{
	/* heap alloc */
	this->pPathHeap   = NULL;
	this->pTrackHeap  = NULL;

	this->pArtistDict = NULL;
	this->pAlbumDict  = NULL;

	this->pHandler    = NULL;

	this->pszBasePath = _strdup(pszFilePath);
	this->err         = 0;
	this->fLoadImages = false;

	this->uTotalFiles     = 0;
	this->uInvalidFiles   = 0;
	this->uProcessedFiles = 0;
	this->uAlbums         = 0;
	this->uArtists        = 0;
	this->uTracks         = 0;
	this->uAllocated      = 0;

	this->pDisposableHeap = NULL;

	this->_tracks = ZLArrayList<ZLID3Track>();

	this->pDefaultAlbum  = (const id3_utf8_t*)"Unknown";
	this->pDefaultArtist = (const id3_utf8_t*)"Unknown";

	this->pUnknownArtist = NULL;
}

ZLID3Library::~ZLID3Library()
{
	delete this->pPathHeap;
	delete this->pTrackHeap;

	delete this->pArtistDict;
	delete this->pAlbumDict;
}

void ZLID3Library::load(ZLID3EventHandler* pHandler)
{
	int ret;

	if (this->pPathHeap != NULL)
	{
		delete this->pPathHeap;
	}
	if (this->pTrackHeap != NULL)
	{
		delete this->pTrackHeap;
	}
	if (this->pArtistDict != NULL)
	{
		delete this->pArtistDict;
	}
	if (this->pAlbumDict != NULL)
	{
		delete this->pAlbumDict;
	}

	this->pPathHeap   = new ZLFastHeap(PATH_HEAP_SIZE);
	this->pTrackHeap  = new ZLFastHeap(TRACK_HEAP_SIZE);
	this->pArtistDict = new ZLFastDict(ARTIST_DICT_SIZE);
	this->pAlbumDict  = new ZLFastDict(ALBUM_DICT_SIZE);

	this->uTotalFiles     = 0;
	this->uInvalidFiles   = 0;
	this->uProcessedFiles = 0;
	this->uAlbums         = 0;
	this->uArtists        = 0;
	this->uTracks         = 0;
	this->uImages         = 0;
	this->uAllocated      = 0;


	this->pHandler = pHandler;

	this->pDisposableHeap = new ZLFastHeap(DISP_HEAP_SIZE);
	ret = _walk(this->pszBasePath, this, ZLID3Library::_countFiles, ZLID3Library::_disposableHeapAlloc);
	delete this->pDisposableHeap;
	this->pDisposableHeap = NULL;

	if (ret != 0 && this->pHandler != NULL)
	{
		this->pHandler->onComplete("Error counting files.", this);
	}

	ret = _walk(this->pszBasePath, this, ZLID3Library::_readFileID3Tag, ZLID3Library::_pathHeapAlloc);
	if (this->pHandler != NULL)
	{
		if (ret != 0)
		{
			this->pHandler->onComplete("Error loading files.", this);
		}
		else
		{
			this->pHandler->onComplete(NULL, this);
		}
	}
}

int ZLID3Library::_countFiles(void* pData, const char* pszBase, const char* pszFile)
{
	const char* pszExt  = _path_file_extension(pszFile);
	ZLID3Library* _THIS = (ZLID3Library*)pData;

	if (pszExt != NULL && strcmp(pszExt, "mp3") == 0)
	{
		_THIS->uTotalFiles++;
	}

	return 0;
}

int ZLID3Library::_readFileID3Tag(void* pData, const char* pszBase, const char* pszFile)
{
	ZLID3Library* _THIS = (ZLID3Library*)pData;
	ZLID3Track*   pTrack = NULL;
 	id3_file*     pID3File;

	const char* pszExt  = _path_file_extension(pszFile);
 	if (pszExt == NULL || strcmp(pszExt, "mp3") != 0)
 	{
 		return 0;
 	}

	char* pszPath = _path_append(pszBase, pszFile, ZLID3Library::_pathHeapAlloc, pData);
 	_THIS->uProcessedFiles++;

	/* get ID3 tags */
	pID3File = id3_file_open(pszPath, ID3_FILE_MODE_READONLY);
	if (pID3File == 0)
	{
		_THIS->uInvalidFiles++;
		_THIS->err = ZL_ERR_OPEN_FD;

		if (_THIS->pHandler != NULL)
		{
			_THIS->pHandler->onLoadFile("Could not load file.", (const char*)pszPath, NULL, _THIS->uTotalFiles);
		}
		return _THIS->err;
	}

	ZLID3Artist* pArtist;
	const id3_utf8_t*  pAlbumName;

	pTrack = new (_THIS->pTrackHeap) ZLID3Track(_THIS->pTrackHeap, pszPath, id3_file_tag(pID3File));
	if (pTrack->artist() != NULL)
	{
		pArtist = (ZLID3Artist*)_THIS->pArtistDict->get(pTrack->artist(), id3_utf8_size(pTrack->artist()));
		if (pArtist == NULL)
		{
			pArtist = new (_THIS->pTrackHeap) ZLID3Artist(pTrack->artist());
			_THIS->pArtistDict->add(pTrack->artist(), id3_utf8_size(pTrack->artist()), pArtist);
			_THIS->uArtists++;
		}
	}
	else
	{
		if (_THIS->pUnknownArtist == NULL)
		{
			_THIS->pUnknownArtist = new (_THIS->pTrackHeap) ZLID3Artist(_THIS->pDefaultArtist);	
		}
		pArtist = _THIS->pUnknownArtist;
	}
	pArtist->addTrack(pTrack);
	pTrack->uid(_THIS->_tracks.push(pTrack));

	pAlbumName = pTrack->album();
	if (pAlbumName == NULL)
	{
		pAlbumName = _THIS->pDefaultAlbum;	
	}

	id3_utf8_t* pAlbumIndex = ZLID3Album::index(_THIS->pTrackHeap, (id3_utf8_t*)pAlbumName, (id3_utf8_t*)pArtist->name());
	ZLID3Album* pAlbum = (ZLID3Album*)_THIS->pAlbumDict->get(pAlbumIndex, id3_utf8_size(pAlbumIndex));
	if (pAlbum == NULL)
	{
		pAlbum = new (_THIS->pTrackHeap) ZLID3Album((id3_utf8_t*)pTrack->album(), (id3_utf8_t*)pArtist->name());
		_THIS->pAlbumDict->add(pAlbumIndex, id3_utf8_size(pAlbumIndex), pAlbum);
		_THIS->uAlbums++;
	}
	pArtist->addAlbum(pAlbum);
	pAlbum->addTrack(pTrack);

	if ( (pAlbumName != _THIS->pDefaultAlbum) && pAlbum->image() == NULL && pTrack->imageFrame() != NULL )
	{
		pAlbum->setImage(_THIS->fLoadImages, pszPath, pTrack->imageFrame());
		_THIS->uImages++;
		_THIS->uAllocated += pAlbum->image()->size();
	}

	_THIS->uTracks++;

	id3_file_close(pID3File);
	
	if (_THIS->pHandler != NULL)
	{
		_THIS->pHandler->onLoadFile(NULL, (const char*)pszPath, pTrack, _THIS->uTotalFiles);
	}

	return 0;
}

void* ZLID3Library::_pathHeapAlloc(void* pData, size_t desired)
{
	ZLID3Library* _THIS = (ZLID3Library*)pData;
	return _THIS->pPathHeap->alloc(desired);
}

void* ZLID3Library::_disposableHeapAlloc(void* pData, size_t desired)
{
	ZLID3Library* _THIS = (ZLID3Library*)pData;
	return _THIS->pDisposableHeap->alloc(desired);
}

void ZLID3Library::dump()
{
	this->pArtistDict->dump();
	this->pAlbumDict->dump();

	printf("---Statistics:\n");
	printf("  Total files found: %u\n", this->uTotalFiles);
	printf("  Invalid files: %u\n", this->uInvalidFiles);
	printf("  Tracks: %u\n", this->uTracks);
	printf("  Albums: %u\n", this->uAlbums);
	printf("  Artists: %u\n", this->uArtists);
	printf("  Images: %u\n", this->uImages);

	printf("---Memory usage:\n");
	printf("  Path Heap: %lu bytes (%lu kb) \n", this->pPathHeap->size(), (size_t)(this->pPathHeap->size()/1024)); 
	printf("  Track Heap: %lu bytes (%lu kb) \n", this->pTrackHeap->size(), (size_t)(this->pTrackHeap->size()/1024)); 
	printf("  Album dict [%u]: %lu bytes (%lu kb) \n", this->pAlbumDict->length(), this->pAlbumDict->size(), (size_t)(this->pAlbumDict->size()/1024)); 
	printf("  Artist dict [%u]: %lu bytes (%lu kb) \n", this->pArtistDict->length(), this->pArtistDict->size(), (size_t)(this->pArtistDict->size()/1024)); 
	printf("  Arrays: %u bytes (%u kb) \n", this->uAllocated, (uint32_t)(this->uAllocated/1024)); 
}

ZLID3Image* ZLID3Library::getAlbumImage(id3_utf8_t* pArtistName, id3_utf8_t* pAlbumName)
{
	id3_utf8_t* pIndex = ZLID3Album::index(NULL, pAlbumName, pArtistName);
	ZLID3Album* pAlbum = (ZLID3Album*)this->pAlbumDict->get(pIndex, id3_utf8_size(pIndex));
	free(pIndex);
	if (pAlbum != NULL && pAlbum->image() != NULL)
	{
		ZLID3Image* pImage = pAlbum->image();
		if (pImage->buffer() == NULL)
		{
			/* get album tracks */
			ZLArrayList<ZLID3Track>* tracks = pAlbum->tracks();
			ZLID3Track* pTrack = tracks->next();
			while (pTrack != NULL)
			{
				if (pTrack->imageFrame() != NULL)
				{
					ZLID3Image* pImage = this->getTrackImage(pTrack->uid());
					if (pImage != NULL)
					{
						pAlbum->setImage(pImage);
						delete pImage;
						return pAlbum->image();
					}
				}
				pTrack = tracks->next();
			}
		}
		else
		{
			return pImage;
		}
	}
	return NULL;
}

ZLID3Image* ZLID3Library::getTrackImage(uint32_t uid)
{
 	id3_file*  pID3File;
 	id3_tag*   pID3Tag;
 	id3_frame* pID3Frame;

	ZLID3Track* pTrack = this->_tracks.elemAt(uid);
	if (pTrack == NULL)
	{
		return NULL;
	}

	/* get ID3 tags */
	pID3File = id3_file_open(pTrack->path(), ID3_FILE_MODE_READONLY);
	if (pID3File == 0)
	{
		return NULL;
	}

	/* search for APIC frame */
	pID3Tag = id3_file_tag(pID3File);
	if (pID3Tag == NULL)
	{
		return NULL;
	}

	pID3Frame = id3_tag_findframe(pID3Tag, "APIC", 0);
	if (pID3Frame == NULL)
	{
		return NULL;
	}

	return new ZLID3Image(true, pTrack->path(), pID3Frame);
}

ZLID3Track* ZLID3Library::getTrack(uint32_t uid)
{
	return this->_tracks.elemAt(uid);
}
