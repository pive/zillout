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

#include <v8.h>
#include <node.h>

using namespace v8;
using namespace node;

/* ZLLibraryWrapper node wrapper
-------------------------------------------------------------------------------
*/
class ZLLibraryWrapper : public node::ObjectWrap
{
public:
	static void Init(v8::Handle<v8::Object> exports);

	void loadLibrary(ZLID3EventHandler* pHandler) { this->pLibrary->load(pHandler); }
	void lock() { this->Ref(); }
	void release() { this->Unref(); }
private:
	ZLID3Library* pLibrary;

	explicit ZLLibraryWrapper(const char* pszBasePath);
	~ZLLibraryWrapper();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Handle<v8::Value> Load(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
};

/* ZLAsyncWorker libuv wrapper
-------------------------------------------------------------------------------
*/

struct _AsyncData_t {
	class ZLAsyncWorker* _THIS;
	void* pUserData;
	uv_cond_t uv_cond;
	uint32_t uid;
};

struct _CallbackData_t {
	int type;
	const char* pszError;
	union {
		struct {
			const char* pszPath;
			class ZLID3Track* pTrack;
			uint32_t    uTotalFiles;
		} load;
		struct {
			class ZLID3Library* pLibrary;
		} complete;
	};
};

class ZLAsyncWorker
{
protected:
	uv_work_t*   request;
	uv_thread_t  v8thread;
	uint32_t     uid;

	ZLArrayList<uv_async_t>* async;

	virtual ~ZLAsyncWorker() {}

	virtual void run() = 0;
	virtual void end() = 0;
	virtual void msg(void*) = 0;

public:
	/* on V8 thread */
	void start()
	{
		this->request = new uv_work_t;
		this->request->data = this;
		this->v8thread = (uv_thread_t)uv_thread_self();
		this->uid = 0;
		this->async = new ZLArrayList<uv_async_t>();
		uv_queue_work(uv_default_loop(), this->request, ZLAsyncWorker::_processRequest, ZLAsyncWorker::_terminateRequest);
	}
	/* on UV thread */
	void callv8(_CallbackData_t* pCallData)
	{
		uv_async_t* pAsync = new uv_async_t;
		_AsyncData_t* pAsyncData = new _AsyncData_t;

		uv_async_init(uv_default_loop(), pAsync, ZLAsyncWorker::_handleAsyncCall);

		pAsyncData->_THIS = this;
		pAsyncData->pUserData = pCallData;
		pAsyncData->uid = ++this->uid;
		pAsync->data = pAsyncData;

		this->async->push(pAsync);

		uv_async_send(pAsync);
 	}
	/* on UV thread */
	void cleanv8()
	{
		usleep(100000); /* let the async v8 finish */
		this->async->reset();
		uv_async_t* pAsync = this->async->next();
		while (pAsync != NULL)
		{
			_AsyncData_t* pAsyncData = (_AsyncData_t*)pAsync->data;
			_CallbackData_t* pCallbackData = (_CallbackData_t*)pAsyncData->pUserData;
			delete pCallbackData;
			delete pAsyncData;
			uv_close((uv_handle_t*) pAsync, NULL);
			pAsync = this->async->next();
		}
		delete this->async;
	}
 	/* on UV thread */
	static void _processRequest(uv_work_t* request)
	{
		ZLAsyncWorker* _THIS = (ZLAsyncWorker*)request->data;
		_THIS->run();
	}
	/* on V8 thread */
	static void _terminateRequest(uv_work_t* request, int val)
	{
		ZLAsyncWorker* _THIS = (ZLAsyncWorker*)request->data;
		_THIS->end();
		delete _THIS->request;
	}
	/* on V8 thread */
	static void _handleAsyncCall(uv_async_t* handle, int status)
	{
		_AsyncData_t* pAsyncData = (_AsyncData_t*)handle->data;
		pAsyncData->_THIS->msg(pAsyncData->pUserData);
	}
};

/* ZLAsyncLoader
-------------------------------------------------------------------------------
*/

class ZLAsyncLoader : public ZLAsyncWorker, ZLID3EventHandler
{
private:
	ZLLibraryWrapper* pWrapper;
	Persistent<Function> onLoadFileCallback;
	Persistent<Function> onCompleteCallback;

public:
	ZLAsyncLoader(ZLLibraryWrapper* pWrapper, Persistent<Function> onLoad, Persistent<Function> onComplete) : pWrapper(pWrapper), onLoadFileCallback(onLoad), onCompleteCallback(onComplete) {}
	virtual ~ZLAsyncLoader();
protected:
	virtual void run();
	virtual void end();
	virtual void msg(void*);
	virtual void onLoadFile(const char* pszError, const char* pszPath, ZLID3Track* pTrack, uint32_t uProcessed);
	virtual void onComplete(const char* pszError, class ZLID3Library* pLibrary);
};
