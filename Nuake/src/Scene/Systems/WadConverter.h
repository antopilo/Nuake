#pragma once

#include <string>

namespace Nuake
{

#define MAKE_RGB(r,g,b)     (uint32_t)(((r) << 16) | ((g) << 8) | (b) | (255<<24))

	// John Carmack said the quake palette.lmp can be considered public domain because it is not an important asset to id, so I include it here as a fallback if no external palette file is found.
		

#define 	CMP_LZSS   1
#define 	CMP_NONE   0
#define 	TYP_LABEL   1
#define 	TYP_LUMPY   64
#define 	TYP_MIPTEX   68
#define 	TYP_NONE   0
#define 	TYP_PALETTE   64
#define 	TYP_QPIC   66
#define 	TYP_QTEX   65
#define 	TYP_SOUND   67

#define MIP_LEVELS 4
	typedef struct miptex_s
	{
		char name[16];
		uint32_t width, height;
		uint32_t offsets[MIP_LEVELS]; // four mip maps stored
	}
	miptex_t;

	typedef struct
	{
		char magic[4];

		uint32_t num_lumps;
		uint32_t dir_start;
	}
	raw_wad2_header_t;

	typedef struct
	{
		uint32_t start;
		uint32_t length;  // compressed
		uint32_t u_len;   // uncompressed

		unsigned char type;
		unsigned char compression;
		unsigned char _pad[2];

		char  name[16];  // must be null terminated
	}
	raw_wad2_lump_t;

#define WAD2_MAGIC  "WAD2"

	// compression method (from Quake1 source)
#define CMP_NONE  0
#define CMP_LZSS  1

// lump types (from Quake1 source)
#define TYP_NONE      0
#define TYP_LABEL     1
#define TYP_PALETTE  64
#define TYP_QTEX     65
#define TYP_QPIC     66
#define TYP_SOUND    67
#define TYP_MIPTEX   68

#define LE_U32(X)  ((uint32_t)(X))

	static FILE *wad_R_fp;
	static raw_wad2_header_t  wad_R_header;
	static raw_wad2_lump_t * wad_R_dir;

	typedef struct
	{
		uint32_t width;
		uint32_t height;

		//  byte pixels[width * height];
	}
	pic_header_t;

	int WAD2_EntryType(int entry);
	bool WAD2_ReadData(int entry, int offset, int length, void *buffer);
	void ExtractWad(const std::string& wadFilePath, const std::string& outputDir);
	bool MIP_ExtractRawBlock(int entry, const char *lump_name);
	bool MIP_ExtractPicture(int entry, const char *lump_name);
	void WAD2_CloseRead(void);
	void WadOpenRead(const std::string& filename);
}