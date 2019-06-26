// class declaration
#pragma once

#include <windows.h>
#include <string>

class ParallelPort
{
public:
    ParallelPort (const std::string &);
	~ParallelPort ();

	/**
	 * Main Port Control Methods
	 */
	void Open();
	void Close();
    std::size_t Read(void * address, std::size_t size);
    std::size_t Write(const void * address, std::size_t size);
	DCB getDCBParameters ();
	void setDCBParameters (const DCB params);
protected:
	// handle to the open port
	HANDLE m_Handle;
    std::string m_Name;
private:
	// hide the default and copy constructors
	ParallelPort ();
	ParallelPort (const ParallelPort&);
	// hide the assignment operator
	ParallelPort& operator= (const ParallelPort&);
};
