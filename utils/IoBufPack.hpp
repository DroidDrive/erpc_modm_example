#pragma once


template<class T>
class IoBufPack{
public:
	IoBufPack(T* in, T* out)
	: receiveBuf_{in}, sendBuf_{out}{}
public:
	T* getReceiveBuffer(){return receiveBuf_;}
	T* getSendBuffer(){return sendBuf_;}
private:
	T* receiveBuf_;
	T* sendBuf_;
};
