#include "stdafx.h"
#include "WaveFile.h"

#define WF_OFFSET_FORMATTAG			20
#define WF_OFFSET_CHANNELS			22
#define WF_OFFSET_SAMPLESPERSEC		24
#define WF_OFFSET_AVGBYTESPERSEC	28
#define WF_OFFSET_BLOCKALIGN		32
#define WF_OFFSET_BITSPERSAMPLE		34
#define WF_OFFSET_DATASIZE			40
#define WF_OFFSET_DATA				44
#define WF_HEADER_SIZE				WF_OFFSET_DATA

wav_file::wav_file()// : m_file(0), m_len(0), m_ChangedFlag(0)
{
	m_file = 0;
	m_len = 0;
	m_ChangedFlag = 0;
	memset( &m_format, 0, sizeof(m_format) );
}

wav_file::~wav_file()
{
	close();
}

int wav_file::create( LPCTSTR name, WAVEFORMATEX *wfx )
{
  errno_t  err;

	if( m_file )
		close();

	DeleteFile( name );

	// open file
	err = _tfopen_s(&m_file, name, TEXT("w+b"));
	
	if(err)
	  return 0;

	memcpy( &m_format, wfx, sizeof(m_format) );

	m_len			= 0;
	m_ChangedFlag	= 0;

	WriteHeaders( &m_format, m_len );

	return 1;
}

int wav_file::create( LPCTSTR name, unsigned char *dat, unsigned int dlen, WAVEFORMATEX *wfx )
{
 errno_t  err;

	if( m_file )
		close();

	err = _tfopen_s(&m_file, name, TEXT("w+b"));

	if(!err)
		return 0;

	m_ChangedFlag	= 0;

	WriteHeaders( wfx, dlen );

	
	if( dat && dlen )
		fwrite( dat, sizeof(dat[0]), dlen,	m_file );

	fclose(m_file);

	m_file = 0;

	return 1;
}
int wav_file::open(LPCTSTR name)
{
 errno_t  errF;
//	unsigned char hdr[WF_HEADER_SIZE];
	
	RIFF_H			RiffH;
	RIFF_CHUNK		RiffC;
//	DATA_H			DataH;

	if( m_file )
		close();

	// open file
	errF = _tfopen_s(&m_file, name, TEXT("rb"));

	if(errF)
		return -1;
	
	m_ChangedFlag = 0;

	// set file length
	fseek( m_file, 0, SEEK_END);
	m_len = ftell(m_file);// - WF_HEADER_SIZE;

	// set the m_format attribute members
	unsigned int count, err(0);

	if( fseek(m_file, 0, SEEK_SET) )
	{
		close();
		return -1;
	}

	fread( &RiffH,		1, sizeof(RIFF_H),	m_file);

	while(1)
	{
		count = fread( &RiffC,	1, sizeof(RIFF_CHUNK),	m_file);

		if( count < sizeof(RIFF_CHUNK) )
		{
			err = -1;
			break;
		}

		if( !memcmp( RiffC.ChunckId, RIFF_FORMAT, 4 ) )
			break;//found

		if( err = fseek( m_file, RiffC.Size, SEEK_CUR ) )
		{
			break;
		}
	}

	if( err )
	{
		close();
		return err;
	}

	count = fread( &m_format, 1, sizeof( WAVEFORMATEX ),	m_file );

	if( count < sizeof( WAVEFORMATEX ) )
	{
		close();
		return err;
	}

	if( err = fseek(m_file, RiffC.Size - sizeof( WAVEFORMATEX ), SEEK_CUR) )
	{
		close();
		return err;
	}

	while(1)
	{
		count = fread( &RiffC,	1, sizeof(RIFF_CHUNK),	m_file);

		if( count < sizeof(RIFF_CHUNK) )
		{
			err = -1;
			break;
		}

		if( !memcmp( RiffC.ChunckId, RIFF_CHANNEL, 4 ) )
			break;//found

		if( err = fseek( m_file, RiffC.Size, SEEK_CUR ) )
		{
			break;
		}
	}
	
	if( m_len < RiffC.Size )
	{
		close();
		return -1;
	}
	else
		m_len = RiffC.Size;

 	return err;
}

void wav_file::WriteHeaders( WAVEFORMATEX* wfx, unsigned int DataLen )
{
	if( m_file && wfx )
	{
		RIFF_H hdr;
		RIFF_CHUNK whdr;
		RIFF_CHUNK dhdr;

		memcpy(whdr.ChunckId, RIFF_FORMAT, sizeof(whdr.ChunckId));
		whdr.Size = sizeof(WAVEFORMATEX) + wfx->cbSize;

		memcpy(dhdr.ChunckId, RIFF_CHANNEL, sizeof(whdr.ChunckId));
		dhdr.Size = DataLen;

		memcpy(hdr.Riff, RIFF_FILE, sizeof(hdr.Riff));
		memcpy(hdr.Wave, RIFF_WAVE, sizeof(hdr.Wave));

		hdr.Size = sizeof(whdr) + whdr.Size + sizeof(dhdr) + dhdr.Size;

		// set file length
		fseek(m_file, 0, SEEK_SET);

		fwrite( &hdr,	1,				sizeof(hdr),	m_file );
		fwrite( &whdr,	1,				sizeof(whdr),	m_file );
		fwrite( wfx,	1,				whdr.Size,		m_file );
		fwrite( &dhdr,	1,				sizeof(dhdr),	m_file );
	}
}
void wav_file::close()
{
	if( m_file )
	{
		if( m_ChangedFlag )
		{
			WriteHeaders( &m_format, m_len );
		}
		
		fclose( m_file );
		m_file = 0;
		memset( &m_format, 0, sizeof(m_format) );
		m_len			= 0;
		m_ChangedFlag	= 0;
	}
}

unsigned int wav_file::get( char *dat, unsigned int llen )
{
	if( m_file )
		return fread(dat, 1, llen, m_file );
	return 0;
}
unsigned int wav_file::put( char *dat, unsigned int llen )
{
	if( m_file )
	{
		if( (size_t)llen == fwrite(dat, 1, llen, m_file ) )
		{
			m_ChangedFlag = 1;
			m_len += llen;
			return llen;
		}
		//else - error
	}
	return 0;
}

unsigned int wav_file::read(char *dat, unsigned int llen, unsigned int offset )
{
	if( offset >= m_len +  WF_HEADER_SIZE )
		return 0;

	fseek( m_file, offset + WF_HEADER_SIZE, SEEK_SET);
	return fread(dat, 1, llen, m_file);
}
