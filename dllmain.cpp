// dllmain.cpp : Define el punto de entrada de la aplicación DLL.
#include "stdafx.h"
#include "winuser.h"
#include <thread>

using namespace std;

#define DllExport __declspec( dllexport )

extern "C" {

DllExport LPSTR __stdcall enumeratePorts();
DllExport WORD __stdcall  open(LPSTR port);
DllExport void __stdcall reportAnalog(WORD channel, WORD enable);
DllExport void __stdcall pinMode(WORD pin, WORD mode);
DllExport void __stdcall digitalWrite(WORD pin, WORD value);
DllExport void __stdcall analogWrite(WORD pin, DWORD value);
DllExport WORD __stdcall digitalRead(WORD pin);
DllExport DWORD __stdcall analogRead(LPSTR channel);

}

std::vector<interfaz::Interfaz> i;
int index = 0;
HANDLE g_hTimer;
int interval = 1;

VOID CALLBACK ParseCallback(PVOID lpParameter, BOOLEAN reserved)
{
	for (std::vector<interfaz::Interfaz>::iterator it = i.begin(); it != i.end(); ++it) {
		(*it).f->parse(); 
	}
}


extern "C" BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		i.reserve(8);
		//CreateTimerQueueTimer(&g_hTimer, NULL, ParseCallback, NULL, interval, interval, 0);
	break;
    case DLL_THREAD_ATTACH:
		//MessageBox(NULL, "DLL_THREAD_ATTACH", "INTERFAZ", 0);
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		//MessageBox(NULL, "Se perdió la conexión con DLL", "INTERFAZ", 0);
		for (std::vector<interfaz::Interfaz>::iterator it = i.begin(); it != i.end(); ++it) {
			(*it).serialio->close();
		}
        break;
    }
    return TRUE;
}

void reconnect(exception e) {
	cout << e.what() << endl;
	i.at(index).serialio->close();
	i.at(index).serialio->open();
}


LPSTR __stdcall enumeratePorts() {
	std::vector<firmata::PortInfo> ports = firmata::FirmSerial::listPorts();
	std::string res("");
	char * str;
	for (auto port : ports) {
		res += port.port + ": " + port.description + '\n';
	}
	str = (char*)malloc(res.size() + 1);
	strcpy_s(str, res.size() + 1, res.c_str());

	return str;
}



WORD __stdcall open(LPSTR port) {
	try {
		interfaz::Interfaz j(port);
		i.push_back(j);
		index = i.size(); 
		std::thread{ &interfaz::Interfaz::parse, j }.detach();
		return index;
	} 
	catch (exception e) {
		cout << e.what() << endl;
		return 0;
	}
}

void __stdcall reportAnalog(WORD channel, WORD enable) {
	try {
		//i.at(index).f->setSamplingInterval(100);
		i.at(index).f->reportAnalog((uint8_t)channel, (uint8_t)enable);
	}
	catch (exception e) {
		reconnect(e);
		reportAnalog(channel, enable);
	}
}

void __stdcall pinMode(WORD pin, WORD mode) {
	try {
		i.at(index).f->pinMode((uint8_t)pin, (uint8_t)mode);
	}
	catch (exception e) {
		reconnect(e);
		pinMode(pin, mode);
	}
}

void __stdcall digitalWrite(WORD pin, WORD value) {
	try {
		i.at(index).f->digitalWrite((uint8_t)pin, (uint8_t)value);
	}
	catch (exception e) {
		reconnect(e);
		digitalWrite(pin, value);
	}
}

void __stdcall analogWrite(WORD pin, DWORD value) {
	try {
		i.at(index).f->analogWrite((uint8_t)pin, (uint32_t)value);
	}
	catch (exception e) {
		reconnect(e);
		analogWrite(pin, value);
	}
}

WORD __stdcall digitalRead(WORD pin) {
	return (WORD)i.at(index).f->digitalRead((uint8_t)pin);
}

DWORD __stdcall analogRead(LPSTR channel) {
	return (DWORD)i.at(index).f->analogRead(channel);
}


