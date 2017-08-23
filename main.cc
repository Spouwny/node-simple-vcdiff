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
	
	void	Reset(v8::Isolate* isolate, v8::Local<v8::Value> val)
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
	bool							success;
	
	InfoEncode() { Reset(); }

	void	Reset()
	{
		callback.Reset();
		bufferSrc.Reset();
		bufferIn.Reset();
		success = false;
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
	
	info->success = decoder.Decode<std::string>(info->bufferSrc.data, info->bufferSrc.len, delta, &info->out);
}


void	DoEncode(uv_work_t* request)
{
	InfoEncode* 				info = (InfoEncode*)request->data;
	open_vcdiff::VCDiffEncoder	encoder(info->bufferSrc.data, info->bufferSrc.len);
 
	//encoder.SetFormatFlags(open_vcdiff::VCD_FORMAT_INTERLEAVED); // no need
	info->success = encoder.Encode<std::string>(info->bufferIn.data, info->bufferIn.len, &info->out);
}

void	CleanInfo(uv_work_t* request, int status)
{
    
	InfoEncode* 	info = (InfoEncode*)request->data;
	v8::HandleScope	scope(info->isolate);

	
	v8::MaybeLocal<v8::Object> buffer;
	v8::Local<v8::Value> data ;
    
	if (info->success)
	{
        
		buffer = node::Buffer::New(info->isolate,info->out.size());
        

		if(buffer.ToLocal(&data)){
			memcpy(node::Buffer::Data(data), info->out.c_str(), info->out.size());

		}
	}
	else
    {
		buffer = node::Buffer::New(info->isolate,0);
		buffer.ToLocal(&data);
	}
    v8::Local<v8::Context> context =info->isolate->GetCurrentContext();
	ToLocal<v8::Function>(&info->callback)
		->Call(
			context,context->Global(),
			1,&data);
	
	info->Reset();
	delete info;
}

void	Encode(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate*	isolate = args.GetIsolate();
	v8::HandleScope	scope(isolate);
  
    if (!CheckArgs(isolate, args)){
        return ;
    }
	InfoEncode*	info = new InfoEncode();
  
	info->isolate = isolate;
    info->request.data = info;
    info->bufferSrc.Reset(isolate, args[0]);
	info->bufferIn.Reset(isolate, args[1]);
    
	info->callback.Reset(isolate, v8::Local<v8::Function>::Cast(args[2]));
    
	uv_queue_work(uv_default_loop(), &info->request, DoEncode, CleanInfo);
	
}
void	sum(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate*	isolate = args.GetIsolate();
    v8::HandleScope	scope(isolate);
    
    int len = args.Length();
    double result = 0;
    int i = 0;
    for( ;i<len;i++){
        v8::Local<v8::Number>  v = args[i]->ToNumber();
        result += v->Value();
    }
    
    args.GetReturnValue().Set(v8::Number::New(isolate, result));
    //return len;
    
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
    NODE_SET_METHOD(exports, "sum", sum);
}

NODE_MODULE(simple_vcdiff, init)
