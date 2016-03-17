#include "mmsystem.h"

typedef void (*SoundEventHandler)(void *sender, void *buffer, void *userData);
void dummy_callback(void*, void*, void*) { };

class Sound
{
	friend LONG CALLBACK waveInProcIN (HWAVEIN hwi, UINT uMsg, Sound *pThis, DWORD dwParam1, DWORD dwParam2);

private:
	WAVEFORMATEX	m_waveFormat;
	HWAVEIN			m_waveInDevice;
	HWAVEOUT		m_waveOutDevice;
	SoundEventHandler	m_onRecorded;
	void				*m_pOnRecordedUserData;
	BOOL			m_stopRequested;
	
public:
	Sound()
	{
		m_pOnRecordedUserData = NULL;
		m_onRecorded = dummy_callback;

		m_waveInDevice = 0;
		m_waveOutDevice = 0;

		m_waveFormat.cbSize = 0;
		m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		m_waveFormat.wBitsPerSample = 16;
		m_waveFormat.nSamplesPerSec = 8000;
		m_waveFormat.nChannels = 1;
		m_waveFormat.nBlockAlign = m_waveFormat.nChannels * m_waveFormat.wBitsPerSample / 8;
		m_waveFormat.nAvgBytesPerSec = m_waveFormat.nBlockAlign * m_waveFormat.nSamplesPerSec;
	}

	~Sound()
	{
		Close();
	}

	void Open()
	{
		MMRESULT res;
		m_stopRequested = FALSE;

		if (!m_waveInDevice)
		{
			res = waveInOpen(&m_waveInDevice, WAVE_MAPPER, &m_waveFormat, (DWORD)&waveInProcIN, (DWORD)this, CALLBACK_FUNCTION);
			if (res != MMSYSERR_NOERROR)
				DebugBreak();
		}

		if (!m_waveOutDevice)
		{
			res = waveOutOpen(&m_waveOutDevice, WAVE_MAPPER, &m_waveFormat, 0, 0, CALLBACK_NULL);
			if (res != MMSYSERR_NOERROR)
				DebugBreak();
		}
	}

	void Close()
	{
		MMRESULT res;
		m_stopRequested = TRUE;

		if (m_waveInDevice)
		{
			res = waveInClose(m_waveInDevice);
			if (res != MMSYSERR_NOERROR)
			{
				if (waveInClose(m_waveInDevice) == WAVERR_STILLPLAYING)
				{
					waveInReset(m_waveInDevice);
					if ((res = waveInClose(m_waveInDevice)) != MMSYSERR_NOERROR)
						DebugBreak();
				}
				else
					DebugBreak();
			}

			m_waveInDevice = 0;
		}

		if (m_waveOutDevice)
		{
			if ((res = waveOutReset(m_waveOutDevice)) != MMSYSERR_NOERROR)
				DebugBreak();

			if ((res = waveOutClose(m_waveOutDevice)) != MMSYSERR_NOERROR)
				DebugBreak();
			else
				m_waveOutDevice = 0;
		}
	}

	void Record(void *buffer)
	{
		MMRESULT res;

		if (m_waveInDevice && buffer && !m_stopRequested)
		{
			WAVEHDR *header = (WAVEHDR *)buffer;
			if (header->dwBytesRecorded)
				waveInUnprepareHeader(m_waveInDevice, header, sizeof(WAVEHDR));

			header->dwBytesRecorded = 0;
			header->dwUser = 0;
			header->dwFlags = 0;
			header->dwLoops = 0;

			res = waveInPrepareHeader(m_waveInDevice, header, sizeof(WAVEHDR));
			if (res != MMSYSERR_NOERROR)
				DebugBreak();

			res = waveInAddBuffer(m_waveInDevice, header, sizeof(WAVEHDR));
			if (res != MMSYSERR_NOERROR)
				DebugBreak();

			res = waveInStart(m_waveInDevice);
			if (res != MMSYSERR_NOERROR)
				DebugBreak();
		}
	}

	void Play(void *buffer)
	{
		if (m_waveOutDevice && buffer && !m_stopRequested)
		{
			WAVEHDR *header = (WAVEHDR *)buffer;
			MMRESULT res = waveOutWrite(m_waveOutDevice, header, sizeof(WAVEHDR));
			if (res != MMSYSERR_NOERROR)
				DebugBreak();
		}
	}

	void *AllocateBuffer(DWORD size)
	{
		WAVEHDR *header = new WAVEHDR();
		if (header)
		{
			memset(header, 0, sizeof(WAVEHDR));
			header->dwBufferLength = size;
			header->lpData = new char[size];

			if (!header->lpData)
			{
				delete(header);
				header = NULL;
			}
		}

		return header;
	}

	void DeallocateBuffer(void *buffer)
	{
		WAVEHDR *header = (WAVEHDR *)buffer;
		if (header)
		{
			delete(header->lpData);
			delete(header);
		}
	}

	void RegisterOnRecorded(SoundEventHandler fn, void *pUserData)
	{
		m_pOnRecordedUserData = pUserData;
		m_onRecorded = fn;
	}
};

LONG CALLBACK waveInProcIN (HWAVEIN hwi, UINT uMsg, Sound *pThis, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == MM_WIM_OPEN)
	{
	}
	else if (uMsg == MM_WIM_CLOSE)
	{
	}
	else if (uMsg == MM_WIM_DATA)
	{
		pThis->m_onRecorded(pThis, (void *)dwParam1, pThis->m_pOnRecordedUserData);
	}
	else
		DebugBreak();

	return 0;
}
