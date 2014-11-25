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
#include "../inc/binding.h"

#define EXCEPTION(m) ThrowException(Exception::Error(String::New(m)))


/* ZLLibrary node wrapper
-------------------------------------------------------------------------------
*/

Persistent<Function> ZLLibraryWrapper::constructor;

ZLLibraryWrapper::ZLLibraryWrapper(const char* pszBasePath)
{
	this->pLibrary = new ZLID3Library(pszBasePath);
}

ZLLibraryWrapper::~ZLLibraryWrapper()
{
	delete this->pLibrary;
}

void ZLLibraryWrapper::Init(Handle<Object> exports)
{
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("ZLID3Library"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->PrototypeTemplate()->Set(String::NewSymbol("load"),FunctionTemplate::New(Load)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getAlbumImage"),FunctionTemplate::New(GetAlbumImage)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getTrackImage"),FunctionTemplate::New(GetTrackImage)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getTracks"),FunctionTemplate::New(GetTracks)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getTrackFile"),FunctionTemplate::New(GetTrackFile)->GetFunction());
	constructor = Persistent<Function>::New(tpl->GetFunction());
	exports->Set(String::NewSymbol("Library"), constructor);
}

Handle<Value> ZLLibraryWrapper::New(const Arguments& args)
{
  HandleScope scope;

  if (args.IsConstructCall())
  {
 	if (args.Length() != 1)
	{
		return EXCEPTION("One argument required: library path is missing.");
	}

	if (!args[0]->IsString())
	{
		return EXCEPTION("First argument must be a string.");
	}

	String::AsciiValue path(args[0]->ToString());
    ZLLibraryWrapper* obj = new ZLLibraryWrapper(*path);
    obj->Wrap(args.This());
    return args.This();

  }
  else
  {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}

Handle<Value> ZLLibraryWrapper::Load(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 3)
	{
		return EXCEPTION("Three arguments required.");
	}

	if (!args[0]->IsFunction())
	{
		return EXCEPTION("First argument must be a callback.");
	}

	if (!args[1]->IsFunction())
	{
		return EXCEPTION("Second argument must be a callback.");
	}

	if (!args[2]->IsFunction())
	{
		return EXCEPTION("Third argument must be a callback.");
	}

	ZLLibraryWrapper* obj = ObjectWrap::Unwrap<ZLLibraryWrapper>(args.This());

	ZLID3Library* pOldLibrary = obj->pLibrary;
	obj->pLibrary = new ZLID3Library(pOldLibrary->path());
	delete pOldLibrary;

	obj->lock();
	
	ZLAsyncLoader* loader = new ZLAsyncLoader(
		obj,
		Persistent<Function>::New(Local<Function>::Cast(args[0])),
		Persistent<Function>::New(Local<Function>::Cast(args[1])),
		Persistent<Function>::New(Local<Function>::Cast(args[2]))
		);

	loader->start();

	return Undefined();
}

Handle<Value> ZLLibraryWrapper::GetAlbumImage(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 2)
	{
		return EXCEPTION("Two arguments required.");
	}

	if (!args[0]->IsString())
	{
		return EXCEPTION("First argument must be a string.");
	}
	if (!args[1]->IsString())
	{
		return EXCEPTION("Second argument must be a string.");
	}

	String::Utf8Value artist(args[0]);
	String::Utf8Value album(args[1]);

	ZLLibraryWrapper* obj = ObjectWrap::Unwrap<ZLLibraryWrapper>(args.This());
	ZLID3Image* pImage    = obj->pLibrary->getAlbumImage((id3_utf8_t*)*artist, (id3_utf8_t*)*album);

	if (pImage == NULL || pImage->buffer() == NULL)
	{
		pImage   = obj->pLibrary->getDefaultImage();
	}

	Buffer *buf = Buffer::New(pImage->size());
	memcpy(Buffer::Data(buf), pImage->buffer(), pImage->size());

	Local<Object> image = Object::New();

	image->Set(String::NewSymbol("type"), String::New(pImage->mime()));
	image->Set(String::NewSymbol("buffer"), buf->handle_);

	return scope.Close(image);
}

Handle<Value> ZLLibraryWrapper::GetTrackImage(const Arguments& args)
{
	HandleScope scope;
	bool fDefault = false;

	if (args.Length() != 1)
	{
		return EXCEPTION("One argument required.");
	}

	if (!args[0]->IsUint32())
	{
		return EXCEPTION("First argument must be a Uint32 value.");
	}

	ZLLibraryWrapper* obj = ObjectWrap::Unwrap<ZLLibraryWrapper>(args.This());
	ZLID3Image* pImage    = obj->pLibrary->getTrackImage(args[0]->ToUint32()->Value());

	if (pImage == NULL || pImage->buffer() == NULL)
	{
		pImage   = obj->pLibrary->getDefaultImage();
		fDefault = true;
	}

	Buffer *buf = Buffer::New(pImage->size());
	memcpy(Buffer::Data(buf), pImage->buffer(), pImage->size());

	Local<Object> image = Object::New();

	image->Set(String::NewSymbol("type"), String::New(pImage->mime()));
	image->Set(String::NewSymbol("buffer"), buf->handle_);

	if (!fDefault)
	{
		delete pImage;
	}

	return scope.Close(image);
}

Handle<Value> ZLLibraryWrapper::GetTracks(const Arguments& args)
{
	HandleScope scope;
	uint32_t start  = 0;
	uint32_t length = 0;

	if (args.Length() > 0)
	{
		if (!args[0]->IsUint32())
		{
			return EXCEPTION("First argument must be a Uint32 value.");
		}
		start = args[0]->ToUint32()->Value();
	}
	if (args.Length() > 1)
	{
		if (!args[1]->IsUint32())
		{
			return EXCEPTION("Second argument must be a Uint32 value.");
		}
		length = args[1]->ToUint32()->Value();
	}

	ZLLibraryWrapper* obj = ObjectWrap::Unwrap<ZLLibraryWrapper>(args.This());
	ZLArrayList<ZLID3Track>* tracks = obj->pLibrary->getTrackList();

	Local<Array> list = Array::New(tracks->length());

	tracks->reset();
	ZLID3Track* pTrack = tracks->next();
	while (pTrack != NULL)
	{
		list->Set(pTrack->uid(), ZLLibraryWrapper::NewTrack(pTrack));
		pTrack = tracks->next();
	}

	return scope.Close(list);
}

Handle<Value> ZLLibraryWrapper::GetTrackFile(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 1)
	{
		return EXCEPTION("One argument required.");
	}

	if (!args[0]->IsUint32())
	{
		return EXCEPTION("First argument must be a Uint32 value.");
	}

	ZLLibraryWrapper* obj = ObjectWrap::Unwrap<ZLLibraryWrapper>(args.This());
	ZLID3Track* pTrack    = obj->pLibrary->getTrack(args[0]->ToUint32()->Value());

	if (pTrack == NULL)
	{
		return Undefined();
	}

	return scope.Close(String::New(pTrack->path()));
}

Handle<Value> ZLLibraryWrapper::NewTrack(ZLID3Track* pTrack)
{
	HandleScope scope;

	Local<Object> obj = Object::New();

	obj->Set(String::NewSymbol("id"), Number::New(pTrack->uid()));

	if (pTrack->artist() != NULL)
		obj->Set(String::NewSymbol("artist"), String::New((const char*)pTrack->artist(), id3_utf8_size(pTrack->artist())-1));
	
	if (pTrack->album() != NULL)
		obj->Set(String::NewSymbol("album"),  String::New((const char*)pTrack->album(), id3_utf8_size(pTrack->album())-1));

	if (pTrack->title() != NULL)
	{
		obj->Set(String::NewSymbol("title"),  String::New((const char*)pTrack->title(), id3_utf8_size(pTrack->title())-1));
	}
	else
	{
		/* make sure it's got the filename for display */
		obj->Set(String::NewSymbol("filename"),  String::New(_path_find_filename(pTrack->path())));
	}

	if (pTrack->tracknum() != NULL) {
		char* stop;
		double d = strtod((const char*)pTrack->tracknum(), &stop);
		obj->Set(String::NewSymbol("number"), Number::New(d));
	}

	if (pTrack->year() != NULL)
		obj->Set(String::NewSymbol("year"),   String::New((const char*)pTrack->year(), id3_utf8_size(pTrack->year())-1));

	return scope.Close(obj);
}

/* ZLAsyncLoader
-------------------------------------------------------------------------------
*/
#define CALLBACK_TYPE_FILE_LOADED         0
#define CALLBACK_TYPE_LIBRARY_LOADED      1
#define CALLBACK_TYPE_ERROR               2

ZLAsyncLoader::~ZLAsyncLoader()
{
}

/* on UV thread */
void ZLAsyncLoader::run()
{
	this->pWrapper->loadLibrary(this);
}

/* on V8 thread */
void ZLAsyncLoader::msg(void* pData)
{
	HandleScope scope;
	_CallbackData_t* pCallbackData = (_CallbackData_t*)pData;

	if (pCallbackData->type == CALLBACK_TYPE_ERROR)
	{
		Handle<Value> argv[1];
		argv[0] = String::New(pCallbackData->pszError);

		TryCatch try_catch;
		this->onErrorCallback->Call(Context::GetCurrent()->Global(), 1, argv);

		if (try_catch.HasCaught())
		{
			FatalException(try_catch);
		}
	}
	else if (pCallbackData->type == CALLBACK_TYPE_FILE_LOADED)
	{
		Handle<Value> argv[3];
		argv[0] = Number::New(pCallbackData->load.uTrackUid);
		argv[1] = Number::New(pCallbackData->load.uProcessedFiles);
		argv[2] = Number::New(pCallbackData->load.uTotalFiles);

		TryCatch try_catch;
		this->onLoadFileCallback->Call(Context::GetCurrent()->Global(), 4, argv);

		if (try_catch.HasCaught())
		{
			FatalException(try_catch);
		}
	}
	else if (pCallbackData->type == CALLBACK_TYPE_LIBRARY_LOADED)
	{
		TryCatch try_catch;
		this->onCompleteCallback->Call(Context::GetCurrent()->Global(), 0, NULL);

		if (try_catch.HasCaught())
		{
			FatalException(try_catch);
		}
	}
}

/* on V8 thread */
void ZLAsyncLoader::end()
{
	this->onLoadFileCallback.Dispose();
	this->onCompleteCallback.Dispose();
	this->pWrapper->release();
}

/* on UV thread */
void ZLAsyncLoader::onLoadFile(const char* pszError, const char* pszPath, ZLID3Track* pTrack, uint32_t uTotalFiles)
{
	_CallbackData_t* pCallbackData = new _CallbackData_t;

	if (pszError != NULL)
	{
		pCallbackData->type     = CALLBACK_TYPE_ERROR;
		pCallbackData->pszError = pszError;

		this->callv8(pCallbackData);
	}
	else
	{
		pCallbackData->type     = CALLBACK_TYPE_FILE_LOADED;
		pCallbackData->pszError = NULL;
		
		pCallbackData->load.uTrackUid       = pTrack->uid();
		pCallbackData->load.uProcessedFiles = ++this->processed;
		pCallbackData->load.uTotalFiles     = uTotalFiles;

		this->callv8(pCallbackData);
	}
}

/* on UV thread */
void ZLAsyncLoader::onComplete(const char* pszError, class ZLID3Library* pLibrary)
{
	_CallbackData_t* pCallbackData = new _CallbackData_t;

	if (pszError != NULL)
	{
		pCallbackData->type     = CALLBACK_TYPE_ERROR;
		pCallbackData->pszError = pszError;

		this->callv8(pCallbackData);
	}
	else
	{
		pCallbackData->type     = CALLBACK_TYPE_LIBRARY_LOADED;
		pCallbackData->pszError = NULL;
		
		pCallbackData->complete.pLibrary = pLibrary;
		this->callv8(pCallbackData);
	}
	this->cleanv8();
}

/* Node js init
-------------------------------------------------------------------------------
*/

void InitAll(Handle<Object> exports)
{
	ZLLibraryWrapper::Init(exports);
}

NODE_MODULE(zlid3, InitAll);
