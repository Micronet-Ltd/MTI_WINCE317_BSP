#pragma once

#include <string>
#include <vector>
#include <queue>
#ifdef WIN32_PLATFORM_PSPC
	#define SecureZeroMemory(a,b) memset(a, 0, b)
#endif
using namespace std;

template<class T> class Singleton
{
public:
   static T& Instance()
    {
		static T instance;
        return instance;
    };

   ~Singleton(){};

protected:
	Singleton(){};
    Singleton(const Singleton& src) {};
};

class Utils : public Singleton<Utils>
{
public:
	Utils();
	~Utils();

public:
	wstring StringToWString(string str);
	string Utils::WStringToString(wstring wstr);
	void Split(string str, string sep, queue<string> &tokens);
	void Split(string str, string sep, string &left, string &right);
	tm GetTimestamp();
	tm CalculateDuration(tm &start, tm &end);
	int GetSecondsFromTimestamp(tm &timestamp);
};

template<class T> class AutoPointer
{
public:
	AutoPointer(T *pt) : m_Pointer(pt) {};
	AutoPointer() : m_Pointer(NULL) {};
	
	AutoPointer(AutoPointer<T>& ref)
	{
		m_Pointer = _strdup(ref.m_Pointer);
	}

	~AutoPointer(){ free(m_Pointer); };
	
	T& operator*() { return *m_Pointer; }
	T* operator&() { return m_Pointer; }
	T* operator->() { return m_Pointer; }
	bool operator==(T* pt) { return m_Pointer == pt; };
	bool operator!=(T* pt) { return m_Pointer != pt; };
	
	AutoPointer<T>& operator=(AutoPointer<T>& ref)
	{
		if (this != &ref) {
			free(m_Pointer);
			m_Pointer = _strdup(ref.m_Pointer);
		}

		return *this;
	}

	AutoPointer<T>& operator=(T *p)
	{
		if (m_Pointer != p) {
			free(m_Pointer);
			m_Pointer = _strdup(p);
		}

		return *this;
	}

protected:
	T* m_Pointer;
};

inline Utils& AfxGetUtils() { return Utils::Instance(); };

class WaitCursor
{
public:
	WaitCursor(HINSTANCE hInst) : m_hInst(hInst) { SetCursor(LoadCursor(m_hInst, IDC_WAIT)); };
	~WaitCursor() { SetCursor(LoadCursor(m_hInst, IDC_ARROW)); };

protected:
	HINSTANCE m_hInst;
};

