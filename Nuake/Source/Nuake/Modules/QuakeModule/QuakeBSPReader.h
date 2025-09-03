#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace BSPParser
{
	unsigned char QuakePalette[768] =
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
	// Structs declaration
	typedef struct					// A Directory entry
	{
		long  offset;				// Offset to entry, in bytes, from start of file
		long  size;                 // Size of entry in file, in bytes
	} dentry_t;

	typedef struct					 // The BSP file header
	{
		long  version;               // Model version, must be 0x17 (23).
		dentry_t entities;           // List of Entities.
		dentry_t planes;             // Map Planes.
		// numplanes = size/sizeof(plane_t)
		dentry_t miptex;             // Wall Textures.
		dentry_t vertices;           // Map Vertices.
		// numvertices = size/sizeof(vertex_t)
		dentry_t visilist;           // Leaves Visibility lists.
		dentry_t nodes;              // BSP Nodes.
		// numnodes = size/sizeof(node_t)
		dentry_t texinfo;            // Texture Info for faces.
		// numtexinfo = size/sizeof(texinfo_t)
		dentry_t faces;              // Faces of each surface.
		// numfaces = size/sizeof(face_t)
		dentry_t lightmaps;          // Wall Light Maps.
		dentry_t clipnodes;          // clip nodes, for Models.
		// numclips = size/sizeof(clipnode_t)
		dentry_t leaves;             // BSP Leaves.
		// numlaves = size/sizeof(leaf_t)
		dentry_t lface;              // List of Faces.
		dentry_t edges;              // Edges of faces.
		// numedges = Size/sizeof(edge_t)
		dentry_t ledges;             // List of Edges.
		dentry_t models;             // List of Models.
		// nummodels = Size/sizeof(model_t)
	} dheader_t;

	typedef float scalar_t;	// Scalar value,

	typedef struct					 // Vector or Position
	{
		scalar_t x;                  // horizontal
		scalar_t y;                  // horizontal
		scalar_t z;                  // vertical
	} vec3_t;

	typedef struct					 // Bounding Box, Float values
	{
		vec3_t   min;                // minimum values of X,Y,Z
		vec3_t   max;                // maximum values of X,Y,Z
	} boundbox_t;

	typedef struct					 // Bounding Box, Short values
	{
		short   min;                 // minimum values of X,Y,Z
		short   max;                 // maximum values of X,Y,Z
	} bboxshort_t;

	typedef struct
	{
		float X;                    // X,Y,Z coordinates of the vertex
		float Y;                    // usually some integer value
		float Z;                    // but coded in floating point
	} vertex_t;

	typedef struct
	{
		unsigned short vertex0;     // index of the start vertex
		//  must be in [0,numvertices[
		unsigned short vertex1;     // index of the end vertex
		//  must be in [0,numvertices[
	} edge_t;

	typedef struct
	{
		vec3_t   vectorS;           // S vector, horizontal in texture space)
		scalar_t distS;             // horizontal offset in texture space
		vec3_t   vectorT;           // T vector, vertical in texture space
		scalar_t distT;             // vertical offset in texture space
		unsigned long   texture_id; // Index of Mip Texture
		//  must be in [0,numtex[
		unsigned long   animated;   // 0 for ordinary textures, 1 for water 
	} surface_t;

	typedef struct
	{
		unsigned short plane_id;    // The plane in which the face lies
		//           must be in [0,numplanes[ 
		unsigned short side;        // 0 if in front of the plane, 1 if behind the plane
		long ledge_id;              // first edge in the List of edges
		//           must be in [0,numledges[
		unsigned short ledge_num;   // number of edges in the List of edges
		unsigned short texinfo_id;  // index of the Texture info the face is part of
		//           must be in [0,numtexinfos[ 
		unsigned char typelight;    // type of lighting, for the face
		unsigned char baselight;    // from 0xFF (dark) to 0 (bright)
		unsigned char light[2];     // two additional light models  
		long lightmap;              // Pointer inside the general light map, or -1
		// this define the start of the face light map
	} face_t;

	typedef struct
	{
		vec3_t normal;              // Vector orthogonal to plane (Nx,Ny,Nz)
		// with Nx2+Ny2+Nz2 = 1
		scalar_t dist;              // Offset to plane, along the normal vector.
		// Distance from (0,0,0) to the plane
		long    type;               // Type of plane, depending on normal vector.
	} plane_t;

	typedef struct					// Mip texture list header
	{
		long numtex;                // Number of textures in Mip Texture list
		long* offset;				// Offset to each of the individual texture
	} mipheader_t;					//  from the beginning of mipheader_t

	typedef struct					// Mip Texture
	{
		char   name[16];            // Name of the texture.
		unsigned long width;        // width of picture, must be a multiple of 8
		unsigned long height;       // height of picture, must be a multiple of 8
		unsigned long offset1;      // offset to u_char Pix[width   * height]
		unsigned long offset2;      // offset to u_char Pix[width/2 * height/2]
		unsigned long offset4;      // offset to u_char Pix[width/4 * height/4]
		unsigned long offset8;      // offset to u_char Pix[width/8 * height/8]
	} miptex_t;

	typedef struct
	{
		boundbox_t bound;			// The bounding box of the Model
		vec3_t origin;				// origin of model, usually (0,0,0)
		long node_id0;				// index of first BSP node
		long node_id1;				// index of the first Clip node
		long node_id2;				// index of the second Clip node
		long node_id3;				// usually zero
		long numleafs;				// number of BSP leaves
		long face_id;				// index of Faces
		long face_num;				// number of Faces
	} model_t;

	struct vec2
	{
		float x;
		float y;
	};

	struct MipTextureData
	{
		std::string name;
		vec2 size;
		std::vector<uint32_t> data;
	};

	// Parsed output, access these after calling Parse().
	std::vector<plane_t> Planes;
	std::vector<vertex_t> Vertices;
	std::vector<miptex_t> MipTextures;
	std::vector<MipTextureData> MipTexturesData;
	std::vector<face_t> Faces;
	std::vector<edge_t> Edges;
	std::vector<int32_t> LEdges;
	std::vector<model_t> Models;
	std::vector<surface_t> TextureInfos;

	// Private data
	std::ifstream bspFile;
	bool _headerParsed = false;
	dheader_t _header;

	int LoadFile(const std::string& path)
	{
		bspFile.open(path, std::ios::in | std::ios::binary);

		if (!bspFile.is_open())
		{
			return -1;
		}

		return 1;
	}

	void UnloadFile()
	{
		bspFile.close();
	}

	template<typename T>
	void Read(T& data)
	{
		bspFile.read((char*)&data, sizeof(T));
	}

	template<typename T>
	void Read(T& data, size_t size)
	{
		bspFile.read((char*)&data, sizeof(T) * size);
	}

	template<typename T>
	void Read(T* data, size_t count)
	{
		bspFile.read(reinterpret_cast<char*>(data), sizeof(T) * count);
	}

	inline void Seek(long offset)
	{
		bspFile.seekg(offset);
	}

	void ParseHeader()
	{
		_headerParsed = true;
		Read(_header);
	}

	void ParsePlanes()
	{
		if (!_headerParsed) ParseHeader();
		Seek(_header.planes.offset);
		uint32_t planeAmount = _header.planes.size / sizeof(plane_t);
		Planes = std::vector<plane_t>(planeAmount);
		Read(Planes[0], planeAmount);
	}

	void ParseTextureInfos()
	{
		if (!_headerParsed) ParseHeader();
		Seek(_header.texinfo.offset);
		uint32_t textureInfoAmount = _header.texinfo.size / sizeof(plane_t);
		TextureInfos = std::vector<surface_t>(textureInfoAmount);
		bspFile.read((char*)TextureInfos.data(), _header.texinfo.size);
	}

	void ParseVertices()
	{
		if (!_headerParsed) ParseHeader();
		Seek(_header.vertices.offset);
		const uint32_t vertexAmount = _header.vertices.size / sizeof(vertex_t);
		Vertices = std::vector<vertex_t>(vertexAmount);
		bspFile.read((char*)Vertices.data(), _header.vertices.size);
	}

	std::vector<uint32_t> GetTextureData(const std::string& textureName)
	{
		auto foundIt = std::find_if(MipTexturesData.begin(), MipTexturesData.end(), [&textureName](const MipTextureData& tex)
			{
				return tex.name == textureName;
			});

		if (foundIt == MipTexturesData.end())
		{
			assert(false && "texture not found in wad");
		}

		return foundIt->data;
	}

	void ParseMips()
	{
		if (!_headerParsed) ParseHeader();
		Seek(_header.miptex.offset);
		long mipTextureAmount;
		Read(mipTextureAmount);

		// offsets are from mipTextureHeader, not from start of file.
		std::vector<long> mipTextureOffsets(mipTextureAmount);
		Read(mipTextureOffsets[0], mipTextureAmount);

		MipTextures = std::vector<miptex_t>(mipTextureAmount);

		for (uint32_t i = 0; i < mipTextureAmount; i++)
		{
			bspFile.seekg(_header.miptex.offset + mipTextureOffsets[i]);

			Read(MipTextures[i]);

			const size_t size = MipTextures[i].width * MipTextures[i].height;
			std::vector<uint32_t> rgbaData(size);
#define MAKE_RGBA(r, g, b, a) ((uint32_t)(((a) << 24) | ((b) << 16) | ((g) << 8) | (r)))
#define MAKE_RGB(r, g, b)     MAKE_RGBA(r, g, b, 255)

			const std::vector<uint8_t> indexedPixels = std::vector<uint8_t>(size);

			// Read buffer
			bspFile.seekg(_header.miptex.offset + mipTextureOffsets[i] + MipTextures[i].offset1);
			bspFile.read((char*)indexedPixels.data(), size);

			for (int y = 0; y < MipTextures[i].height; y++)
			{
				for (int x = 0; x < MipTextures[i].width; x++)
				{
					unsigned char index = indexedPixels[y * MipTextures[i].width + x];
					unsigned char r = QuakePalette[index * 3 + 0];
					unsigned char g = QuakePalette[index * 3 + 1];
					unsigned char b = QuakePalette[index * 3 + 2];
					unsigned char a = (index == 255) ? 0 : 255; // Index 255 = transparent

					rgbaData[y * MipTextures[i].width + x] = MAKE_RGBA(r, g, b, a);
				}
			}

			MipTexturesData.push_back({ MipTextures[i].name, { (float)MipTextures[i].width, (float)MipTextures[i].height }, rgbaData });
		}
	}

	void ParseFaces()
	{
		if (!_headerParsed) ParseHeader();
		Seek(_header.faces.offset);
		uint32_t faceAmount = _header.faces.size / sizeof(face_t);
		Faces = std::vector<face_t>(faceAmount);
		Read(Faces[0], faceAmount);
	}

	void ParseEdges()
	{
		if (!_headerParsed) ParseHeader();
		Seek(_header.edges.offset);
		uint32_t edgesAmount = _header.edges.size / sizeof(edge_t);
		Edges = std::vector<edge_t>(edgesAmount);
		Read(Edges[0], edgesAmount);
	}

	void ParseLedges()
	{
		if (!_headerParsed) ParseHeader();
		Seek(_header.ledges.offset);
		LEdges = std::vector<int32_t>(_header.ledges.size / sizeof(short));
		bspFile.read((char*)LEdges.data(), _header.ledges.size);
	}

	void ParseModels()
	{
		if (!_headerParsed) ParseHeader();

		Seek(_header.models.offset);
		uint32_t modelAmount = _header.models.size / sizeof(model_t);
		Models = std::vector<model_t>(modelAmount);
		bspFile.read((char*)Models.data(), _header.models.size);
	}

	void Parse()
	{
		ParseHeader();
		ParsePlanes();
		ParseTextureInfos();
		ParseFaces();
		ParseEdges();
		ParseLedges();
		ParseModels();
		ParseVertices();
		ParseMips();
	}
}