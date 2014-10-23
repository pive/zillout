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

	if (args.Length() != 2)
	{
		return EXCEPTION("Two arguments required.");
	}

	if (!args[0]->IsFunction())
	{
		return EXCEPTION("First argument must be a callback.");
	}

	if (!args[1]->IsFunction())
	{
		return EXCEPTION("Second argument must be a callback.");
	}

	ZLLibraryWrapper* obj = ObjectWrap::Unwrap<ZLLibraryWrapper>(args.This());
	obj->lock();
	
	ZLAsyncLoader* loader = new ZLAsyncLoader(
		obj,
		Persistent<Function>::New(Local<Function>::Cast(args[0])),
		Persistent<Function>::New(Local<Function>::Cast(args[1]))
		);

	fprintf(stderr, "Start loader...\n");
	loader->start();

	return Undefined();
}


/* ZLAsyncLoader
-------------------------------------------------------------------------------
*/
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
	if (pCallbackData->type == 0)
	{
		Handle<Value> argv[4];

		argv[0] = Undefined();
		argv[1] = String::New(pCallbackData->load.pszPath);
		argv[2] = Undefined();
		argv[3] = Number::New(pCallbackData->load.uTotalFiles);

		TryCatch try_catch;
		this->onLoadFileCallback->Call(Context::GetCurrent()->Global(), 4, argv);

		if (try_catch.HasCaught())
		{
			FatalException(try_catch);
		}
	}
	else
	{
		Handle<Value> argv[2];

		argv[0] = Undefined();
		argv[1] = Undefined();

		TryCatch try_catch;
		this->onCompleteCallback->Call(Context::GetCurrent()->Global(), 2, argv);

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
	pCallbackData->type = 0;
	pCallbackData->pszError = pszError;
	pCallbackData->load.pszPath = pszPath;
	pCallbackData->load.pTrack = pTrack;
	pCallbackData->load.uTotalFiles = uTotalFiles;

	this->callv8(pCallbackData);
}

/* on UV thread */
void ZLAsyncLoader::onComplete(const char* pszError, class ZLID3Library* pLibrary)
{
	_CallbackData_t* pCallbackData = new _CallbackData_t;
	pCallbackData->type = 1;
	pCallbackData->pszError = pszError;
	pCallbackData->complete.pLibrary = pLibrary;

	this->callv8(pCallbackData);
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
