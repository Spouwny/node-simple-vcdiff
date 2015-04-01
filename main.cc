// hello.cc
#include <node.h>
#include <node_buffer.h>
#include <uv.h>

#include <string.h>
#include <iostream>

#include "open_vcdiff/src/google/vcencoder.h"
#include "open_vcdiff/src/google/vcdecoder.h"

struct PersistentBuffer
{
	v8::Persistent<v8::Value> 		buffer;
	char*							data;
	size_t							len;
	
	void	Reset(v8::Isolate* isolate, v8::Local<v8::Value>& val)
	{
		buffer.Reset(isolate, val);
		data = node::Buffer::Data(val);
		len = node::Buffer::Length(val);
	}
	
	void	Reset()
	{
		data = NULL;
		len = 0;
		buffer.Reset();
	}
};

struct InfoEncode
{
	v8::Isolate* 					isolate;
	uv_work_t						request;
	v8::Persistent<v8::Function>	callback;
	PersistentBuffer 				bufferSrc;
	PersistentBuffer 				bufferIn;
	std::string						out;
	
	void	Reset()
	{
		callback.Reset();
		bufferSrc.Reset();
		bufferIn.Reset();
	}
};

template<class T>
inline v8::Local<T> ToLocal(v8::Persistent<T>* p_) {
  return *reinterpret_cast<v8::Local<T>*>(p_);
}

void	ThrowTypeError(v8::Isolate* isolate, const char* message)
{
	isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, message)));
}

void	ThrowBadNumberArguments(v8::Isolate* isolate)
{
	return ThrowTypeError(isolate, "Wrong number of arguments");
}

bool	CheckArgs(v8::Isolate* isolate, const v8::FunctionCallbackInfo<v8::Value>& args)
{
 	if (args.Length() != 3)
		return ThrowBadNumberArguments(isolate), false;
	if (!node::Buffer::HasInstance(args[0]))
		return ThrowTypeError(isolate, "args[0] should be a buffer."), false;
	if (!node::Buffer::HasInstance(args[1]))
		return ThrowTypeError(isolate, "args[1] should be a buffer."), false;
	if (!args[2]->IsFunction())
		return ThrowTypeError(isolate, "args[2] should be a function."), false;
	return true ;
}

void	DoDecode(uv_work_t* request)
{
	InfoEncode* 				info = (InfoEncode*)request->data;
	open_vcdiff::VCDiffDecoder	decoder;
	std::string					delta(info->bufferIn.data, info->bufferIn.len);
	
	decoder.Decode<std::string>(info->bufferSrc.data, info->bufferSrc.len, delta, &info->out);
}


void	DoEncode(uv_work_t* request)
{
	InfoEncode* 				info = (InfoEncode*)request->data;
	open_vcdiff::VCDiffEncoder	encoder(info->bufferSrc.data, info->bufferSrc.len);
	
	//encoder.SetFormatFlags(open_vcdiff::VCD_FORMAT_INTERLEAVED); // no need
	encoder.Encode<std::string>(info->bufferIn.data, info->bufferIn.len, &info->out);
}

void	CleanInfo(uv_work_t* request, int status)
{
	InfoEncode* 	info = (InfoEncode*)request->data;
	v8::HandleScope	scope(info->isolate);

	
	v8::Local<v8::Object> buffer = node::Buffer::New(info->out.size());
	memcpy(node::Buffer::Data(buffer), info->out.c_str(), info->out.size());
	
	const unsigned argc = 1;
	v8::Local<v8::Value> argv[argc] = { buffer };
	
	ToLocal<v8::Function>(&info->callback)->Call(info->isolate->GetCurrentContext()->Global(), argc, argv);
	
	info->Reset();
	delete info;
}

void	Encode(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate*	isolate = args.GetIsolate();
	v8::HandleScope	scope(isolate);
  
	if (!CheckArgs(isolate, args))
		return ;
	
	InfoEncode*	info = new InfoEncode();
  
	info->isolate = isolate;
	info->request.data = info;
	info->bufferSrc.Reset(isolate, args[0]);
	info->bufferIn.Reset(isolate, args[1]);
	info->callback.Reset(isolate, v8::Local<v8::Function>::Cast(args[2]));
	
	uv_queue_work(uv_default_loop(), &info->request, DoEncode, CleanInfo);
	
}


void	Decode(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate*	isolate = args.GetIsolate();
	v8::HandleScope	scope(isolate);
  
	if (!CheckArgs(isolate, args))
		return ;
	
	InfoEncode*	info = new InfoEncode();
  
	info->isolate = isolate;
	info->request.data = info;
	info->bufferSrc.Reset(isolate, args[0]);
	info->bufferIn.Reset(isolate, args[1]);
	info->callback.Reset(isolate, v8::Local<v8::Function>::Cast(args[2]));
	
	uv_queue_work(uv_default_loop(), &info->request, DoDecode, CleanInfo);
	
}
void	init(v8::Handle<v8::Object> exports)
{
	NODE_SET_METHOD(exports, "Encode", Encode);
	NODE_SET_METHOD(exports, "Decode", Decode);
}

NODE_MODULE(simple_vcdiff, init)