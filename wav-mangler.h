struct wav_t{
  char                RIFF[4];        
  int                 ChunkSize;     
  char                WAVE[4];       
  char                fmt[4];        
  int                 Subchunk1Size;                              
  short int           AudioFormat;  
  short int           NumChan;      
  int                 SamplesPerSec;  
  int                 bytesPerSec;    
  short int           blockAlign;    
  short int           bitsPerSample;  
  char               *extra;
  char                Subchunk2ID[4];
  int                 Subchunk2Size; 
  short int          *data;
};

typedef struct wav_t WAV;

WAV *read_file(char *wavfile);
void hi_pass(WAV *inwav, char *outfile, int freq);
void lo_pass(WAV *inwav, char *outfile, int freq);
void sound_bite(WAV *inwav, WAV *bitewav, char *outfile, double time);
void noise_cancel(WAV *inwav, WAV *noisewav, char *outfile, double time);
void roundSong(WAV *inwav, char *outfile);
