#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "prog10.h"
  
#define PI (3.141592653589793)

WAV wav_struct;
float r, f; // resolution, cutoff frequency
float a1, a2, a3, b1, b2, c; // values to compute hi/lo pass
long extra_size; // length of the extra array
long sizeOfFile; // # samples in the file
// Char arrays to hold different parts of the file that are loaded into the struct
unsigned char *BE_ChunkSize;
unsigned char *BE_Subchunk1Size;
unsigned char *BE_AudioFormat;
unsigned char *BE_NumChan;
unsigned char *BE_SampleRate;
unsigned char *BE_ByteRate;
unsigned char *BE_BlockAlign;
unsigned char *BE_BitsPerSample;
unsigned char *extra;
unsigned char *BE_Subchunk2Size;

WAV *read_file(char *wavfile){

	long i,j,k;
	long data_loc = 0;
	char *fileContents; // contents of the file 
  	
	FILE *wav_file = fopen(wavfile, "r"); // file with the wav data

	// RIFF
	fscanf(wav_file, "%4s", &wav_struct.RIFF[0]); // fill out riff in struct

	// ChunkSize
	fseek(wav_file, 4, SEEK_SET); // move to right spot in file
	BE_ChunkSize = malloc(4*sizeof(char)); // init chunksize
	int ChunkSize = 0; // test --> chunksize = 0 means error
	fscanf(wav_file, "%4s", &BE_ChunkSize[0]); // scan in the chunk size 
	for(i=0;i<4;i++) ChunkSize += (int)BE_ChunkSize[i]*(int)(pow(256,i)); // convert from little to big endian
	wav_struct.ChunkSize = ChunkSize; // store into the struct

	// Everything following is following a similar process to RIFF or Chunksize

	// WAVE
	fseek(wav_file, 8, SEEK_SET);
	fscanf(wav_file, "%4s", &wav_struct.WAVE[0]);

	// FMT
	fseek(wav_file, 12, SEEK_SET);
	fscanf(wav_file, "%4s", &wav_struct.fmt[0]);

	// Subchunk1Size
	fseek(wav_file, 16, SEEK_SET);
	BE_Subchunk1Size = malloc(4*sizeof(char));
	int Subchunk1Size = 0;
	fscanf(wav_file, "%4s", &BE_Subchunk1Size[0]);
	for(i=0;i<4;i++) Subchunk1Size += (int)BE_Subchunk1Size[i]*(int)(pow(256,i));
	wav_struct.Subchunk1Size = Subchunk1Size;

	// Audio Format
	fseek(wav_file, 20, SEEK_SET);
	BE_AudioFormat = malloc(2*sizeof(char));
	fscanf(wav_file, "%2s", &BE_AudioFormat[0]);
	int AudioFormat = 0;
	for(i=0;i<2;i++) AudioFormat += (int)BE_AudioFormat[i]*(int)(pow(256,i));
	wav_struct.AudioFormat = AudioFormat;

	// Num Channels
	fseek(wav_file, 22, SEEK_SET);
	BE_NumChan = malloc(2*sizeof(char));
	fscanf(wav_file, "%2s", &BE_NumChan[0]);
	int NumChan = 0;
	for(i=0;i<2;i++) NumChan += (int)BE_NumChan[i]*(int)(pow(256,i));
	wav_struct.NumChan = NumChan;

	// SampleRate
	fseek(wav_file, 24, SEEK_SET);
	BE_SampleRate = malloc(4*sizeof(char));
	int SampleRate = 0;
	fscanf(wav_file, "%4s", &BE_SampleRate[0]);
	for(i=0;i<4;i++) SampleRate += BE_SampleRate[i]*(int)(pow(256,i));
	wav_struct.SamplesPerSec = SampleRate;

	// ByteRate
	fseek(wav_file, 28, SEEK_SET);
	BE_ByteRate = malloc(4*sizeof(char));
	int ByteRate = 0;
	fscanf(wav_file, "%4s", &BE_ByteRate[0]);
	for(i=0;i<4;i++) ByteRate += (int)(BE_ByteRate[i]*(pow(256,i)));
	wav_struct.bytesPerSec = ByteRate;
	
	// BlockAlign
	fseek(wav_file, 32, SEEK_SET);
	BE_BlockAlign = malloc(2*sizeof(char));
	fscanf(wav_file, "%2s", &BE_BlockAlign[0]);
	int BlockAlign = 0;
	for(i=0;i<2;i++) BlockAlign += (int)BE_BlockAlign[i]*(int)(pow(256,i));
	wav_struct.blockAlign = BlockAlign;

	// BitsPerSample
	fseek(wav_file, 34, SEEK_SET);
	BE_BitsPerSample = malloc(2*sizeof(char));
	fscanf(wav_file, "%2s", &BE_BitsPerSample[0]);
	int BitsPerSample = 0;
	for(i=0;i<2;i++) BitsPerSample += (int)BE_BitsPerSample[i]*(int)(pow(256,i));
	wav_struct.bitsPerSample = BitsPerSample;

	// extra
	extra_size = Subchunk1Size - 16;
	printf("extra size = %li\n", extra_size);
	if(extra_size > 0) extra = malloc(extra_size * sizeof(char));

	// Subchunk2ID
	fseek(wav_file, 36+extra_size, SEEK_SET);
	fscanf(wav_file, "%4s", &wav_struct.Subchunk2ID[0]);

	// Subchunk2Size
	fseek(wav_file, 40 + extra_size, SEEK_SET);
	BE_Subchunk2Size = malloc(4*sizeof(char));
	int Subchunk2Size = 0;
	fscanf(wav_file, "%4s", &BE_Subchunk2Size[0]);
	for(i=0;i<4;i++) Subchunk2Size += (int)(BE_Subchunk2Size[i]*(pow(256,i)));
	wav_struct.Subchunk2Size = Subchunk2Size;


	// data
	fseek(wav_file, 44+extra_size, SEEK_SET);
	sizeOfFile = (Subchunk2Size*8) / (BitsPerSample); // calculate the # samples
	short int *data = malloc(sizeOfFile * sizeof(short int)); // init data array

	for(i=0;i<sizeOfFile;i++) {
		unsigned char *sample = malloc(BitsPerSample); // char value of sample
		int sample_value = 0; // value to increment before storing
		for(j=0;j<BitsPerSample / 8;j++) fscanf(wav_file, "%1s", &sample[j]);	// do 1 %1s per character
		for(k=0;k<BitsPerSample / 8;k++) sample_value += (int)(sample[k]*pow(16,k)); // convert from little  endian to big endian
		data[i] = sample_value; // store into data array
	}
 
	wav_struct.data = data; // store into struct

	return &wav_struct; // return filled out struct
}

void hi_pass(WAV *inwav, char *outfile, int freq){
	int i,j,k;

	r = 0.5; // resolution
	f = freq; // input freq

	// Given computational values
	c = tan(PI * f / inwav->SamplesPerSec);	 
	a1 = 1.0 / ( 1.0 + r * c + c * c);
	a2 = -2.0 * a1;
	a3 = a1;
	b1 = 2.0 * ( c*c - 1.0) * a1;
	b2 = ( 1.0 - r * c + c * c) * a1;

	FILE *outFile = fopen(outfile, "w"); // output file to write to

	// Write header to out file
	// RIFF
	fwrite(&inwav->RIFF[0], sizeof(char), 4, outFile);
	// ChunkSize
	fwrite(BE_ChunkSize, sizeof(char), 4, outFile);
	// WAVE
	fwrite(&inwav->WAVE[i], sizeof(char), 4, outFile);
	// fmt
	fwrite("fmt ", sizeof(char), 4, outFile);
	// Subchunk1Size
	fwrite(BE_Subchunk1Size, sizeof(char), 4, outFile);
	// Audio Format
	fwrite(BE_AudioFormat, sizeof(char), 2, outFile);
	// NumChan
	fwrite(BE_NumChan, sizeof(char), 2, outFile);
	// Sample Rate
	fwrite(BE_SampleRate, sizeof(char), 4, outFile);
	// Byte Rate
	fwrite(BE_ByteRate, sizeof(char), 4, outFile);
	// Block Align
	fwrite(BE_BlockAlign, sizeof(char), 2, outFile);
	// Bits Per Sample
	fwrite(BE_BitsPerSample, sizeof(char), 2, outFile);
	// extra
	fwrite(extra, sizeof(char), extra_size, outFile);
	// Subchunk2ID
	fwrite(&inwav->Subchunk2ID[0], sizeof(char), 4, outFile);
	// Subchunk2Size
	fwrite(BE_Subchunk2Size, sizeof(char), 4, outFile);
	// Subchunk2ID
	for(i=0;i<4;i++) fprintf(outFile, "%x ", inwav->Subchunk2ID[i]);

	// Manipulate data
	short int *data = inwav->data;
	unsigned char *data_out = malloc(sizeOfFile * sizeof(char));
	for(i=0;i<sizeOfFile;i++) data_out[i] = 0; // Init data out to zeroes

	for(i=0;i<sizeOfFile;i++) {
		if(i%2==0) {
			// Left channel (even)
			if(i==0) {// Won't have i-1*2, i-2*2 | i-1*2, i-2*2
				data_out[i] = a1*data[i];
			}
			else if (i==2) { // Won't have i-2*2 | i-2*2
				data_out[i] = a1*data[i] + a2*data[i-1*2] - b1*data_out[i-1*2];
			}
			else if(i>=4) // Normal case 
				data_out[i] = a1*data[i] + a2*data[i-1*2] + a3*data[i-2*2] - b1*data_out[i-1*2] - b2*data_out[i-2*2];
			else ;
		}
		else {
			// Right channel (odd)
			if(i==1) // Won't have i-1*2, i-2*2 | i-1*2, i-2*2
				data_out[i] = a1*data[i];
			else if (i==3) // Won't have i-2*2 | i-2*2
				data_out[i] = a1*data[i] + a2*data[i-1*2] - b1*data_out[i-1*2];
			else if(i>=5) // Normal case 
				data_out[i] = a1*data[i] + a2*data[i-1*2] + a3*data[i-2*2] - b1*data_out[i-1*2] - b2*data_out[i-2*2];
			else ;
		}
	}

	unsigned char *data_out_mod = malloc(sizeOfFile * sizeof(char));
	for(i=0;i<sizeOfFile;i++) data_out_mod[i] = 0;

	// Swap bits for little/big endian
	for(i=0;i<sizeOfFile;i+=inwav->bitsPerSample/4) { // iterate through data out

		//if(i<10)printf("%i-->[%x]\n", i, data_out[j]);
		// Handle even case
		for(j=i+inwav->bitsPerSample/4;j>=i;j-=2) { // write data[j]...data[i] (little endian is big endian backwards)
						data_out_mod[i] = data_out[j];
			//fprintf(outFile,"%x", data_out[j]);
  			//fwrite(&data_out[j], sizeof(int), sizeof(data_out[j]), outFile);
		}
		// Handle odd case
		for(j=1+i+inwav->bitsPerSample/4;j>=i;j-=2) {
			data_out_mod[i/2] = data_out[j];
			//fprintf(outFile,"%x", data_out[j]);
			// fwrite(&data_out[j], sizeof(int), sizeof(data_out[j]), outFile);
		}
	}

	for(i=0;i<sizeOfFile;i++) printf("%i-->%x\n", i, data_out_mod[i]);
	for(i=0;i<sizeOfFile;i++) fwrite(&data_out_mod[i], sizeof(int), sizeof(data_out_mod[i]), outFile);

	fclose(outFile); // close the outfile


}

void lo_pass(WAV *inwav, char *outfile, int freq){
	long i,j,k;

	r = 0.5; // resolution
	f = freq;

	
	c = 1.0 / tan( PI * f / inwav->SamplesPerSec);
	 
	a1 = 1.0 / ( 1.0 + r * c + c * c);
	a2 = 2.0 * a1;
	a3 = a1;
	b1 = 2.0 * ( 1.0 - c*c) * a1;
	b2 = ( 1.0 - r * c + c * c) * a1;


	FILE *outFile = fopen(outfile, "w"); // output file to write to

	// Write header to out file
	// RIFF
	fwrite(&inwav->RIFF[0], sizeof(char), 4, outFile);
	// ChunkSize
	fwrite(BE_ChunkSize, sizeof(char), 4, outFile);
	// WAVE
	fwrite(&inwav->WAVE[i], sizeof(char), 4, outFile);
	// fmt
	fwrite("fmt ", sizeof(char), 4, outFile);
	// Subchunk1Size
	fwrite(BE_Subchunk1Size, sizeof(char), 4, outFile);
	// Audio Format
	fwrite(BE_AudioFormat, sizeof(char), 2, outFile);
	// NumChan
	fwrite(BE_NumChan, sizeof(char), 2, outFile);
	// Sample Rate
	fwrite(BE_SampleRate, sizeof(char), 4, outFile);
	// Byte Rate
	fwrite(BE_ByteRate, sizeof(char), 4, outFile);
	// Block Align
	fwrite(BE_BlockAlign, sizeof(char), 2, outFile);
	// Bits Per Sample
	fwrite(BE_BitsPerSample, sizeof(char), 2, outFile);
	// extra
	fwrite(extra, sizeof(char), extra_size, outFile);
	// Subchunk2ID
	fwrite(&inwav->Subchunk2ID[0], sizeof(char), 4, outFile);
	// Subchunk2Size
	fwrite(BE_Subchunk2Size, sizeof(char), 4, outFile);
	// Subchunk2ID
	for(i=0;i<4;i++) fprintf(outFile, "%x ", inwav->Subchunk2ID[i]);

	// Manipulate data
	short int *data = inwav->data;
	unsigned char *data_out = malloc(sizeOfFile * sizeof(char));
	for(i=0;i<sizeOfFile;i++) data_out[i] = 0; // Init data out to zeroes

	for(i=0;i<sizeOfFile;i++) {
		if(i%2==0) {
			// Left channel (even)
			if(i==0) {// Won't have i-1*2, i-2*2 | i-1*2, i-2*2
				data_out[i] = a1*data[i];
			}
			else if (i==2) { // Won't have i-2*2 | i-2*2
				data_out[i] = a1*data[i] + a2*data[i-1*2] - b1*data_out[i-1*2];
			}
			else if(i>=4) // Normal case 
				data_out[i] = a1*data[i] + a2*data[i-1*2] + a3*data[i-2*2] - b1*data_out[i-1*2] - b2*data_out[i-2*2];
			else ;
		}
		else {
			// Right channel (odd)
			if(i==1) // Won't have i-1*2, i-2*2 | i-1*2, i-2*2
				data_out[i] = a1*data[i];
			else if (i==3) // Won't have i-2*2 | i-2*2
				data_out[i] = a1*data[i] + a2*data[i-1*2] - b1*data_out[i-1*2];
			else if(i>=5) // Normal case 
				data_out[i] = a1*data[i] + a2*data[i-1*2] + a3*data[i-2*2] - b1*data_out[i-1*2] - b2*data_out[i-2*2];
			else ;
		}
	}

	unsigned char *data_out_mod = malloc(sizeOfFile * sizeof(char));
	for(i=0;i<sizeOfFile;i++) data_out_mod[i] = 0;

	// Swap bits for little/big endian
	for(i=0;i<sizeOfFile;i+=inwav->bitsPerSample/4) { // iterate through data out

		//if(i<10)printf("%i-->[%x]\n", i, data_out[j]);
		// Handle even case
		for(j=i+inwav->bitsPerSample/4;j>=i;j-=2) { // write data[j]...data[i] (little endian is big endian backwards)
						data_out_mod[i] = data_out[j];
			//fprintf(outFile,"%x", data_out[j]);
  			//fwrite(&data_out[j], sizeof(int), sizeof(data_out[j]), outFile);
		}
		// Handle odd case
		for(j=1+i+inwav->bitsPerSample/4;j>=i;j-=2) {
			data_out_mod[i/2] = data_out[j];
			//fprintf(outFile,"%x", data_out[j]);
			// fwrite(&data_out[j], sizeof(int), sizeof(data_out[j]), outFile);
		}
	}

//	for(i=0;i<sizeOfFile;i++) printf("%i-->%x\n", i, data_out_mod[i]);
	for(i=0;i<sizeOfFile;i++) fwrite(&data_out_mod[i], sizeof(int), sizeof(data_out_mod[i]), outFile);

	fclose(outFile); // close the outfile

}

// TODO: implement
void noise_cancel(WAV *inwav, WAV *noisewav, char *outfile, double time){
	int i,j,k;
	
	FILE *outFile = fopen(outfile, "w");

	// Write header to out file

	// RIFF
	for(i=0;i<4;i++) fprintf(outFile, "%x", inwav->RIFF[i]);
	// ChunkSize
	for(i=0;i<4;i++) fprintf(outFile, "%.2x", BE_ChunkSize[i]);
	// WAVE
	for(i=0;i<4;i++) fprintf(outFile, "%x", inwav->WAVE[i]);
	// fmt
	for(i=0;i<3;i++) fprintf(outFile, "%x", inwav->fmt[i]);
	fprintf(outFile, "%s", "20");
	// Subchunk1Size
	for(i=0;i<4;i++) fprintf(outFile, "%.2x", BE_Subchunk1Size[i]);
	// Audio Format
	for(i=0;i<2;i++) fprintf(outFile, "%.2x", BE_AudioFormat[i]);
	// NumChan
	for(i=0;i<2;i++) fprintf(outFile, "%.2x", BE_NumChan[i]);
	// Sample Rate
	for(i=0;i<4;i++) fprintf(outFile, "%.2x", BE_SampleRate[i]);
	// Byte Rate
	for(i=0;i<4;i++) fprintf(outFile, "%.2x", BE_ByteRate[i]);
	// Block Align
	for(i=0;i<2;i++) fprintf(outFile, "%.2x", BE_BlockAlign[i]);
	// Bits Per Sample
	for(i=0;i<2;i++) fprintf(outFile, "%.2x", BE_BitsPerSample[i]);
	// extra
	for(i=0;i<extra_size;i++) fprintf(outFile, "%.2x", extra[i]);
	// Subchunk2ID
	for(i=0;i<4;i++) fprintf(outFile, "%x", inwav->Subchunk2ID[i]);

	// Manipulate data


	fclose(outFile);
}


