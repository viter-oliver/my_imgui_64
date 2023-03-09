#pragma once
#include<functional>
#include <thread>
using namespace auto_future;
DWORD WINAPI ThreadLoadApps(LPVOID lpParameter);

class scomm
{
	HANDLE _hcomm = { INVALID_HANDLE_VALUE };
	int _nmb;
	int _baudrate;
#ifdef _OVER_LAP_OP
	OVERLAPPED m_osReader;
	OVERLAPPED m_osWriter;
#endif
	uint8_t calc_check_sum(uint8_t * pBuf, int iLen)
	{
		int i = 0;
		uint8_t check_sum = 0;
		check_sum = pBuf[0];
		for (i = 1; i < iLen; i++)
		{
			check_sum ^= pBuf[i];
		}
		return check_sum;
	}
public:
	scomm()
	{}
	~scomm()
	{
		if (_hcomm != INVALID_HANDLE_VALUE)
		{
			PurgeComm(_hcomm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_TXABORT | PURGE_RXABORT);
			unsigned long etat;
			ClearCommError(_hcomm, &etat, NULL);
			CloseHandle(_hcomm);
		}
	}
	HANDLE open(int nmb, int baudrate)
	{
		_nmb = nmb;
		_baudrate = baudrate;
		char comm_str[50] = { 0 };
		sprintf(comm_str, "COM%d", _nmb);
		_hcomm = CreateFile(comm_str, GENERIC_READ | GENERIC_WRITE, 0, NULL,OPEN_EXISTING,
#ifdef _OVER_LAP_OP
			FILE_FLAG_OVERLAPPED, 
#else
			NULL,
#endif 
			NULL);
		if (_hcomm == INVALID_HANDLE_VALUE)
		{
			printf("fail to open%s!\n", comm_str);
			return INVALID_HANDLE_VALUE;
		}
#ifdef _OVER_LAP_OP
		FillMemory(&m_osReader, sizeof(OVERLAPPED), 0);
		FillMemory(&m_osWriter, sizeof(OVERLAPPED), 0);
		m_osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_osWriter.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif
		int byteUsedTime = 14400 / baudrate + 1;
		COMMTIMEOUTS timeouts = { 20 + byteUsedTime, byteUsedTime, 1000, byteUsedTime, 20 };
		if (!SetCommTimeouts(_hcomm, &timeouts))
		{
			printf("fail to SetCommTimeouts!\n");
			return INVALID_HANDLE_VALUE;
		}
		DCB commParam;
		if (!GetCommState(_hcomm, &commParam))
		{
			printf("fail to get commstate!\n");
			return INVALID_HANDLE_VALUE;
		}
		commParam.BaudRate = _baudrate;				// 设置波特率 

		commParam.fBinary = TRUE;					// 设置二进制模式，此处必须设置TRUE
		commParam.fParity = FALSE;					// 支持奇偶校验 
		commParam.ByteSize = 8;						// 数据位,范围:4-8 
		commParam.Parity = NOPARITY;				// 校验模式
		//commParam.StopBits = ONESTOPBIT;			// 停止位 

		//commParam.fOutxCtsFlow = FALSE;				// No CTS output flow control 
		//commParam.fOutxDsrFlow = FALSE;				// No DSR output flow control 
		commParam.fDtrControl = 0;
		//// DTR flow control type 
		//commParam.fDsrSensitivity = FALSE;			// DSR sensitivity 
		commParam.fTXContinueOnXoff = 0;			// XOFF continues Tx 
		commParam.fOutX = FALSE;					// No XON/XOFF out flow control 
		commParam.fInX = FALSE;						// No XON/XOFF in flow control 
		//commParam.fErrorChar = FALSE;				// Disable error replacement 
		//commParam.fNull = FALSE;					// Disable null stripping 
		commParam.fRtsControl = 0;
		// RTS flow control 
		commParam.fAbortOnError = FALSE;			// 当串口发生错误，并不终止串口读写

		if (!SetCommState(_hcomm, &commParam))
		{
			printf(" fail to setCommState!\n");
			return INVALID_HANDLE_VALUE;
		}
		//SetupComm(_hcomm, 0x400, 0x400);
	

		//指定端口监测的事件集
		SetCommMask(_hcomm, EV_RXCHAR);

		//分配设备缓冲区
		::SetupComm(_hcomm, 4096, 4096);

		//初始化缓冲区中的信息
		::PurgeComm(_hcomm, PURGE_TXCLEAR | PURGE_RXCLEAR);
		return _hcomm;
	}


	DWORD ReadData(LPVOID lpBuf, DWORD dwToRead)
	{
		//TRACE("RRRRRRRRRRRR 00\n"); 
		if (_hcomm == INVALID_HANDLE_VALUE) return 0;
		DWORD dwRead;
		if (ReadFile(_hcomm, lpBuf, dwToRead, &dwRead,
#ifdef _OVER_LAP_OP
			&m_osReader
#else
			NULL
#endif
			))
			return dwRead;
		else
			return 0;
#ifdef _OVER_LAP_OP
		if (GetLastError() != ERROR_IO_PENDING)  return 0;

		if (WaitForSingleObject(m_osReader.hEvent, INFINITE) != WAIT_OBJECT_0)
			return 0;
		if (!GetOverlappedResult(_hcomm, &m_osReader, &dwRead, FALSE))
			return 0;
		//TRACE("RRRRRRRRRRRR 11\n");  
		return dwRead;
#endif
	}
	
};
