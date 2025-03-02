#include "WadConverter.h"

#include "Nuake/Core/Logger.h"
#include "Nuake/Core/String.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Vendors/stb_image/stb_image_write.h"
#include <filesystem>
#include "Nuake/Rendering/Textures/Material.h"

namespace Nuake
{
	struct ConvertedTexture
	{
		std::string path;
		bool fullbright;
	};

	std::string TargetDirectory = "";
	std::string WadName = "";
	std::vector<ConvertedTexture> ConvertedTextures;
	
	unsigned char host_quakepal[768] =
	{
		// marked: colormap colors: cb = (colormap & 0xF0);cb += (cb >= 128 && cb < 224) ? 4 : 12;
		// 0x0*
		0,0,0,        15,15,15,     31,31,31,     47,47,47,     63,63,63,     75,75,75,     91,91,91,     107,107,107,
		123,123,123,  139,139,139,  155,155,155,  171,171,171,  187,187,187,  203,203,203,  219,219,219,  235,235,235,
		// 0x1*                                                   0 ^
		15,11,7,      23,15,11,     31,23,11,     39,27,15,     47,35,19,     55,43,23,     63,47,23,     75,55,27,
		83,59,27,     91,67,31,     99,75,31,     107,83,31,    115,87,31,    123,95,35,    131,103,35,   143,111,35,
		// 0x2*                                                   1 ^
		11,11,15,     19,19,27,     27,27,39,     39,39,51,     47,47,63,     55,55,75,     63,63,87,     71,71,103,
		79,79,115,    91,91,127,    99,99,139,    107,107,151,  115,115,163,  123,123,175,  131,131,187,  139,139,203,
		// 0x3*                                                   2 ^
		0,0,0,        7,7,0,        11,11,0,      19,19,0,      27,27,0,      35,35,0,      43,43,7,      47,47,7,
		55,55,7,      63,63,7,      71,71,7,      75,75,11,     83,83,11,     91,91,11,     99,99,11,     107,107,15,
		// 0x4*                                                   3 ^
		7,0,0,        15,0,0,       23,0,0,       31,0,0,       39,0,0,       47,0,0,       55,0,0,       63,0,0,
		71,0,0,       79,0,0,       87,0,0,       95,0,0,       103,0,0,      111,0,0,      119,0,0,      127,0,0,
		// 0x5*                                                   4 ^
		19,19,0,      27,27,0,      35,35,0,      47,43,0,      55,47,0,      67,55,0,      75,59,7,      87,67,7,
		95,71,7,      107,75,11,    119,83,15,    131,87,19,    139,91,19,    151,95,27,    163,99,31,    175,103,35,
		// 0x6*                                                   5 ^
		35,19,7,      47,23,11,     59,31,15,     75,35,19,     87,43,23,     99,47,31,     115,55,35,    127,59,43,
		143,67,51,    159,79,51,    175,99,47,    191,119,47,   207,143,43,   223,171,39,   239,203,31,   255,243,27,
		// 0x7*                                                   6 ^
		11,7,0,       27,19,0,      43,35,15,     55,43,19,     71,51,27,     83,55,35,     99,63,43,     111,71,51,
		127,83,63,    139,95,71,    155,107,83,   167,123,95,   183,135,107,  195,147,123,  211,163,139,  227,179,151,
		// 0x8*                                                   7 ^        v 8
			171,139,163,  159,127,151,  147,115,135,  139,103,123,  127,91,111,   119,83,99,    107,75,87,    95,63,75,
			87,55,67,     75,47,55,     67,39,47,     55,31,35,     43,23,27,     35,19,19,     23,11,11,     15,7,7,
			// 0x9*                                                   9 v
				187,115,159,  175,107,143,  163,95,131,   151,87,119,   139,79,107,   127,75,95,    115,67,83,    107,59,75,
				95,51,63,     83,43,55,     71,35,43,     59,31,35,     47,23,27,     35,19,19,     23,11,11,     15,7,7,
				// 0xA*                                                  10 v
					219,195,187,  203,179,167,  191,163,155,  175,151,139,  163,135,123,  151,123,111,  135,111,95,   123,99,83,
					107,87,71,    95,75,59,     83,63,51,     67,51,39,     55,43,31,     39,31,23,     27,19,15,     15,11,7,
					// 0xB*                                                  11 v
		111,131,123,  103,123,111,  95,115,103,   87,107,95,    79,99,87,     71,91,79,     63,83,71,     55,75,63,
		47,67,55,     43,59,47,     35,51,39,     31,43,31,     23,35,23,     15,27,19,     11,19,11,     7,11,7,
		// 0xC*                                                  12 v
		255,243,27,   239,223,23,   219,203,19,   203,183,15,   187,167,15,   171,151,11,   155,131,7,    139,115,7,
		123,99,7,     107,83,0,     91,71,0,      75,55,0,      59,43,0,      43,31,0,      27,15,0,      11,7,0,
		// 0xD*                                                  13 v
			0,0,255,      11,11,239,    19,19,223,    27,27,207,    35,35,191,    43,43,175,    47,47,159,    47,47,143,
			47,47,127,    47,47,111,    47,47,95,     43,43,79,     35,35,63,     27,27,47,     19,19,31,     11,11,15,
			// 0xE*
				43,0,0,       59,0,0,       75,7,0,       95,7,0,       111,15,0,     127,23,7,     147,31,7,     163,39,11,
				183,51,15,    195,75,27,    207,99,43,    219,127,59,   227,151,79,   231,171,95,   239,191,119,  247,211,139,
				// 0xF*                                                  14 ^
									167,123,59,   183,155,55,   199,195,55,   231,227,87,   127,191,255,  171,231,255,  215,255,255,  103,0,0,
									139,0,0,      179,0,0,      215,0,0,      255,0,0,      255,243,147,  255,247,199,  255,255,255,  159,91,83
	}; //     

	int WAD2_EntryType(int entry)
	{
		//ASSERT(entry >= 0 && entry < (int)wad_R_header.num_lumps);

		if (wad_R_dir[entry].compression != 0)
			return 0;

		return wad_R_dir[entry].type;
	}

	uint32_t COL_ReadPalette(unsigned char pix)
	{
		unsigned char R = host_quakepal[pix * 3 + 0];
		unsigned char G = host_quakepal[pix * 3 + 1];
		unsigned char B = host_quakepal[pix * 3 + 2];

		return MAKE_RGB(R, G, B);
	}

	bool WAD2_ReadData(int entry, int offset, int length, void *buffer)
	{
		raw_wad2_lump_t *L = &wad_R_dir[entry];

		if (fseek(wad_R_fp, L->start + offset, SEEK_SET) != 0)
			return false;

		int res = fread(buffer, length, 1, wad_R_fp);

		return (res == 1);
	}

	static const char * ExpandFileName(const char *lump_name, bool fullbright)
	{
		int max_len = strlen(lump_name) + 32;

		char *result = (char *)calloc(max_len + 1, 1); (max_len);

		// convert any special first character
		if (lump_name[0] == '*')
		{
			strcpy(result, "star_");
		}
		else if (lump_name[0] == '+')
		{
			strcpy(result, "plus_");
		}
		else if (lump_name[0] == '-')
		{
			strcpy(result, "minu_");
		}
		else if (lump_name[0] == '/')
		{
			strcpy(result, "divd_");
		}
		else if (lump_name[0] == '{')
		{
			strcpy(result, "bra_");
		}
		else
		{
			result[0] = lump_name[0];
			result[1] = 0;
		}

		strcat(result, lump_name + 1);

		// sanitize filename (remove problematic characters)
		bool warned = false;

		for (char *p = result; *p; p++)
		{
			if (*p == ' ')
				*p = '_';

			if (*p != '_' && *p != '-' && !isalnum(*p))
			{
				if (!warned)
				{
					printf("WARNING: removing weird characters from name (\\%03o)\n",
						   (unsigned char)*p);
					warned = true;
				}

				*p = '_';
			}
		}

		//if (fullbright)
		//	strcat(result, "_fbr");

		strcat(result, ".png");

		return result;
	}

	bool MIP_ExtractRawBlock(int entry, const char *lump_name)
	{
		int total = wad_R_dir[entry].u_len;

		// guess size
		int width, height;

		if (lump_name == "CONCHARS")
		{
			width = 128;
			height = 128;
		}
		else if (lump_name == "TINYFONT")
		{
			width = 128;
			height = 32;
		}
		else
		{
			for (width = 4096; width * width > total; width /= 2)
			{
			}

			height = width;
		}

		printf("  Guessing size to be: %dx%d\n", width, height);

		if (width < 8 || width  > 2048 ||
			height < 8 || height > 2048)
		{
			printf("FAILURE: weird size of picture: %dx%d\n\n", width, height);
			return false;
		}

		unsigned char* pixels = new unsigned char[width * height];

		if (!WAD2_ReadData(entry, 0, width * height, pixels))
		{
			printf("FAILURE: could not read %dx%d pixels from picture\n\n", width, height);
			delete[] pixels;
			return false;
		}

		stbi_write_png(ExpandFileName(lump_name, false), width, height, 3, pixels, sizeof(unsigned char) * 3);
		
		delete[] pixels;

		return true;
	}

	bool MIP_ExtractPicture(int entry, const char *lump_name)
	{
		pic_header_t pic;

		if (!WAD2_ReadData(entry, 0, (int)sizeof(pic), &pic))
		{
			printf("FAILURE: could not read picture header!\n\n");
			return false;
		}

		int width = pic.width;
		int height = pic.height;

		if (width < 1 || width  > 2048 ||
			height < 1 || height > 2048)
		{
			printf("FAILURE: weird size of picture: %dx%d\n\n", width, height);
			return false;
		}

		char* pixels = new char[width * height];

		if (!WAD2_ReadData(entry, (int)sizeof(pic), width * height, pixels))
		{
			printf("FAILURE: could not read %dx%d pixels from picture\n\n", width, height);
			delete[] pixels;
			return false;
		}

		// create the image for saving.
		
		std::vector<uint32_t> textureData;
		textureData.reserve(width * height);
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				unsigned char pix = pixels[y * width + x];

				textureData[y * width + x] = COL_ReadPalette(pix);
			}


		stbi_write_png(ExpandFileName(lump_name, false), width, height, 3, textureData.data(), sizeof(uint32_t));


		delete[] pixels;

		return true;
	}

	bool MIP_ExtractMipTex(int entry, const char *lump_name)
	{
		// mip header
		miptex_t mm_tex;

		if (!WAD2_ReadData(entry, 0, (int)sizeof(mm_tex), &mm_tex))
		{
			printf("FAILURE: could not read miptex header!\n\n");
			return false;
		}

		// (We ignore the internal name and offsets)

		mm_tex.width = LE_U32(mm_tex.width);
		mm_tex.height = LE_U32(mm_tex.height);

		int width = mm_tex.width;
		int height = mm_tex.height;

		if (width < 8 || width  > 4096 ||
			height < 8 || height > 4096)
		{
			printf("FAILURE: weird size of image: %dx%d\n\n", width, height);
			return false;
		}

		unsigned char *pixels = new unsigned char[width * height];

		// NOTE: we assume that the pixels directly follow the miptex header

		if (!WAD2_ReadData(entry, (int)sizeof(mm_tex), width * height, pixels))
		{
			printf("FAILURE: could not read %dx%d pixels from miptex\n\n", width, height);
			delete[] pixels;
			return false;
		}

		// create the image for saving.
		// if the image contains fullbright pixels, the output filename
		// will be given the '_fbr' prefix.
		bool fullbright = false;
		std::vector<uint32_t> textureData;
		textureData.resize(width * height);
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				unsigned char pix = pixels[y * width + x];

				if (pix >= 256 - 32)
				{
					fullbright = true;
				}

				textureData[y * width + x] = COL_ReadPalette(pix);
			}
		}

		const std::string lumpName = ExpandFileName(lump_name, fullbright);
		const std::string finalFilePath = TargetDirectory + lumpName;
		stbi_write_png((FileSystem::Root + finalFilePath).c_str(), width, height, 4, textureData.data(), width * 4);

		delete[] pixels;

		ConvertedTextures.push_back({ std::string(lumpName), fullbright });

		return true;
	}

	void ExtractWad(const std::string& wadPath, const std::string& targetDirectory)
	{
		TargetDirectory = targetDirectory;
		if (wadPath.size() == 0)
		{
			Logger::Log("Failed to extract wad. Path was empty", "wadExtractor");
			return;
		}

		if (!String::EndsWith(wadPath, ".wad"))
		{
			Logger::Log("File must have .wad extension", "wadExtractor");
			return;
		}

		ConvertedTextures = std::vector<ConvertedTexture>();

		auto pathSplits = String::Split(std::string(wadPath.begin(), wadPath.end() - 4), '/');
		pathSplits = String::Split(pathSplits[std::size(pathSplits) - 1], '\\');
		WadName = pathSplits[std::size(pathSplits) - 1];
		TargetDirectory = "/Textures/" + WadName + "/";

		if (!FileSystem::DirectoryExists("/Textures/"))
		{
			FileSystem::MakeDirectory("/Textures/");
		}

		if (!FileSystem::DirectoryExists(TargetDirectory))
		{
			FileSystem::MakeDirectory(TargetDirectory);
		}

		WadOpenRead(wadPath);

		printf("\n");
		printf("--------------------------------------------------\n");

		int num_lumps = wad_R_header.num_lumps;

		int skipped = 0;
		int failures = 0;

		for (int i = 0; i < num_lumps; i++)
		{
			int type = WAD2_EntryType(i);
			const char *name = wad_R_dir[i].name;

			// special handling for two odd-ball lumps (raw pixels)
			if (name == "CONCHARS" ||
				name == "TINYFONT")
			{
				printf("Unpacking %d/%d (BLOCK) : %s\n", i + 1, num_lumps, name);

				MIP_ExtractRawBlock(i, name);
			}
			else if (type == TYP_QPIC)
			{
				printf("Unpacking %d/%d (PIC) : %s\n", i + 1, num_lumps, name);

				if (!MIP_ExtractPicture(i, name))
					failures++;
			}
			else if (type == TYP_MIPTEX)
			{
				printf("Unpacking %d/%d : %s\n", i + 1, num_lumps, name);

				if (!MIP_ExtractMipTex(i, name))
					failures++;
			}
			else
			{
				printf("SKIPPING NON-MIPTEX entry %d/%d : %s\n", i + 1, num_lumps, name);
				skipped++;
				continue;
			}
		}

		printf("--------------------------------------------------\n");
		printf("\n");

		WAD2_CloseRead();

		if (skipped > 0)
			printf("Skipped %d non-miptex lumps\n", skipped);

		printf("Extracted %d entries, with %d failures\n",
			   num_lumps - failures - skipped, failures);

		CreateMaterials();

		ConvertedTextures.clear();

	}

	void CreateMaterials()
	{
		const std::string materialFolderName = "/Materials";
		const std::string& materialFolderPath = materialFolderName + "/" + WadName + "/";

		if(!FileSystem::DirectoryExists(materialFolderPath))
		{
			if (!FileSystem::DirectoryExists(materialFolderName))
			{
				FileSystem::MakeDirectory(materialFolderName);
			}

			FileSystem::MakeDirectory(materialFolderPath);
		}

		for (auto& t : ConvertedTextures)
		{
			Ref<Material> material = CreateRef<Material>();

			if (t.fullbright)
			{
				material->SetUnlit(true);
				material->data.u_Emissive = 2.0f;
			}

			material->SetAlbedo(TextureManager::Get()->GetTexture(FileSystem::RelativeToAbsolute(TargetDirectory + t.path)));

			auto jsonData = material->Serialize();

			const std::string materialFilePath = materialFolderPath + std::string(t.path.begin(), t.path.end() - 4) + ".material";

			FileSystem::BeginWriteFile(materialFilePath);
			FileSystem::WriteLine(jsonData.dump(4));
			FileSystem::EndWriteFile();
		}
	}

	void WadOpenRead(const std::string& filename)
	{
		wad_R_fp = fopen(filename.c_str(), "rb");

		if (!wad_R_fp)
		{
			printf("WAD2_OpenRead: no such file: %s\n", filename.c_str());
			return;
		}

		printf("Opened WAD2 file: %s\n", filename.c_str());

		if (fread(&wad_R_header, sizeof(wad_R_header), 1, wad_R_fp) != 1)
		{
			printf("WAD2_OpenRead: failed reading header\n");
			fclose(wad_R_fp);
			return;
		}

		if (memcmp(wad_R_header.magic, WAD2_MAGIC, 4) != 0)
		{
			printf("WAD2_OpenRead: not a WAD2 file!\n");
			fclose(wad_R_fp);
			return;
		}

		wad_R_header.num_lumps = LE_U32(wad_R_header.num_lumps);
		wad_R_header.dir_start = LE_U32(wad_R_header.dir_start);

		if (wad_R_header.num_lumps >= 5000)  // sanity check
		{
			printf("WAD2_OpenRead: bad header (%d entries?)\n", wad_R_header.num_lumps);
			fclose(wad_R_fp);
			return;
		}

		if (fseek(wad_R_fp, wad_R_header.dir_start, SEEK_SET) != 0)
		{
			printf("WAD2_OpenRead: cannot seek to directory (at 0x%08x)\n", wad_R_header.dir_start);
			fclose(wad_R_fp);
			return;
		}

		wad_R_dir = new raw_wad2_lump_t[wad_R_header.num_lumps + 1];
		for (int i = 0; i < (int)wad_R_header.num_lumps; i++)
		{
			raw_wad2_lump_t *L = &wad_R_dir[i];

			int res = fread(L, sizeof(raw_wad2_lump_t), 1, wad_R_fp);

			if (res == EOF || res != 1 || ferror(wad_R_fp))
			{
				if (i == 0)
				{
					printf("WAD2_OpenRead: could not read any dir-entries!\n");

					delete[] wad_R_dir;
					wad_R_dir = NULL;

					fclose(wad_R_fp);
					return;
				}

				printf("WAD2_OpenRead: hit EOF reading dir-entry %d\n", i);

				// truncate directory
				wad_R_header.num_lumps = i;
				break;
			}

			// make sure name is NUL terminated.
			L->name[15] = 0;

			L->start = LE_U32(L->start);
			L->length = LE_U32(L->length);
			L->u_len = LE_U32(L->u_len);

			//    DebugPrintf(" %4d: %08x %08x : %s\n", i, L->start, L->length, L->name);
		}
	}

	void WAD2_CloseRead(void)
	{
		fclose(wad_R_fp);

		printf("Closed WAD2 file\n");

		delete[] wad_R_dir;
		wad_R_dir = NULL;
	}
	
}
