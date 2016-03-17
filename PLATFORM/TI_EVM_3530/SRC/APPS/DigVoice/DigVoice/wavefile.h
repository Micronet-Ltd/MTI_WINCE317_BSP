
#ifndef _WAVE_FILE_H
#define _WAVE_FILE_H

#pragma pack(push, 1)
typedef struct tagRiff_h
{
   char Riff[4];		// Chunk ID RIFF
   unsigned Size;		// Chunk Data Size
   char Wave[4];		// RIFF type ID WAVE.
}RIFF_H;

typedef struct
{
   char		ChunckId[4];        // chunk ID fmt.
   unsigned Size;      // Chunk Data Size
}RIFF_CHUNK;

#define RIFF_FILE       "RIFF"
#define RIFF_WAVE       "WAVE"
#define RIFF_FORMAT     "fmt "
#define RIFF_CHANNEL    "data"

class wav_file
{
public:
	wav_file();
	virtual ~wav_file();

	int		open(LPCTSTR name);
	int		create(LPCTSTR name, WAVEFORMATEX *wfx);
	int		create(LPCTSTR name, unsigned char *dat, unsigned int dlen, WAVEFORMATEX *wfx);
	void	close(void);

	unsigned int	get	( char *dat, unsigned int len	) ;
	unsigned int	put	( char *dat, unsigned int len	) ;
	unsigned int	read( char *dat, unsigned int len, unsigned int offset = 0  ) ;
	unsigned int	length(void){return m_len;};

	WAVEFORMATEX *get_format(void){return (&m_format);};

protected:
	void	WriteHeaders( WAVEFORMATEX* wfx, unsigned int DataLen );

	FILE*	m_file;
	unsigned int	m_len;
	WAVEFORMATEX	m_format;
	int				m_ChangedFlag;
};
#pragma pack(pop)

#endif // _WAVE_FILE_H