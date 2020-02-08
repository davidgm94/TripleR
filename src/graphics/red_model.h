#include <indexgenerator.cpp>
#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj.h>

typedef struct Vertex
{
	f16 vx, vy, vz, vw;
	u8 nx, ny, nz, nw;
	f16 tu, tv;
} Vertex;

typedef struct
{
	u32 vertices[64];
	u8 indices[126 * 3]; // up to 42 triangles
	u8 triangleCount;
	u8 vertexCount;
} Meshlet;

typedef struct
{
	vector<Vertex> vertices;
	vector<u32> indices;
	vector<Meshlet> meshlets;
} Mesh;

static inline Mesh loadObj(const char* path)
{
	fastObjMesh* obj = fast_obj_read(path);

	size_t indexCount = 0;

	for (u32 i = 0; i < obj->face_count; i++)
	{
		indexCount += 3 * (obj->face_vertices[i] - 2);
	}

	vector<Vertex> vertices(indexCount);

	u64 vertexOffset = 0;
	u64 indexOffset = 0;

	for (u32 i = 0; i < obj->face_count; i++)
	{
		for (u32 j = 0; j < obj->face_vertices[i]; j++)
		{
			fastObjIndex gi = obj->indices[indexOffset + j];
			if (j >= 3)
			{
				vertices[vertexOffset + 0] = vertices[vertexOffset - 3];
				vertices[vertexOffset + 1] = vertices[vertexOffset - 1];
				vertexOffset += 2;
			}

			Vertex v;
			v.vx = meshopt_quantizeHalf(obj->positions[gi.p * 3 + 0]);
			v.vy = meshopt_quantizeHalf(obj->positions[gi.p * 3 + 1]);
			v.vz = meshopt_quantizeHalf(obj->positions[gi.p * 3 + 2]);

			v.nx = (u8)(obj->normals[gi.n * 3 + 0] * 127.f + 127.5f);
			v.ny = (u8)(obj->normals[gi.n * 3 + 1] * 127.f + 127.5f);
			v.nz = (u8)(obj->normals[gi.n * 3 + 2] * 127.f + 127.5f);
			
			v.tu = meshopt_quantizeHalf(obj->texcoords[gi.t * 2 + 0]);
			v.tv = meshopt_quantizeHalf(obj->texcoords[gi.t * 2 + 1]); // originally: obj->texcoords[gi.t * 2 + 1]

			vertices[vertexOffset] = v;
			vertexOffset++;
		}

		indexOffset += obj->face_vertices[i];
	}

	os_assert(vertexOffset == indexCount);
	fast_obj_destroy(obj);

	Mesh mesh;
    vector<u32> remap(indexCount);
    u64 vertexCount = meshopt_generateVertexRemap(remap.data(), null, indexCount, vertices.data(), indexCount, sizeof(Vertex));

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);

    meshopt_remapVertexBuffer(mesh.vertices.data(), vertices.data(), indexCount, sizeof(Vertex), remap.data());
    meshopt_remapIndexBuffer(mesh.indices.data(), null, indexCount, remap.data());

    meshopt_optimizeVertexCache(mesh.indices.data(), mesh.indices.data(), indexCount, vertexCount);
    meshopt_optimizeVertexFetch(mesh.vertices.data(), mesh.indices.data(), indexCount, mesh.vertices.data(), vertexCount, sizeof(Vertex));

	return mesh;
}

void buildMeshlets(Mesh* mesh)
{
	Meshlet meshlet = {};

    const u64 meshIndexCount = mesh->indices.size();
    const u64 meshVertexCount = mesh->vertices.size();
	vector<u8> meshletVertices(meshVertexCount, 0xff);

	for (u64 i = 0; i < meshIndexCount; i+= 3)
	{
		u64 a = mesh->indices[i + 0];
		u64 b = mesh->indices[i + 1];
		u64 c = mesh->indices[i + 2];

		u8* av = &meshletVertices[a];
		u8* bv = &meshletVertices[b];
		u8* cv = &meshletVertices[c];

		if (meshlet.vertexCount + (*av == 0xff) + (*bv == 0xff) + (*cv == 0xff) > 64 || meshlet.triangleCount >= 126)
		{
		    mesh->meshlets.push_back(meshlet);
			for (u64 j = 0; j < meshlet.vertexCount; ++j)
			{
				meshletVertices[meshlet.vertices[j]] = 0xff;
			}
			os_memset(&meshlet, 0, sizeof(Meshlet));
		}

		if (*av == 0xff)
		{
			*av = meshlet.vertexCount;
			meshlet.vertices[meshlet.vertexCount++] = a;
		}
		if (*bv == 0xff)
		{
			*bv = meshlet.vertexCount;
			meshlet.vertices[meshlet.vertexCount++] = b;
		}
		if (*cv == 0xff)
		{
			*cv = meshlet.vertexCount;
			meshlet.vertices[meshlet.vertexCount++] = c;
		}

		meshlet.indices[meshlet.triangleCount * 3 + 0] = *av;
        meshlet.indices[meshlet.triangleCount * 3 + 1] = *bv;
        meshlet.indices[meshlet.triangleCount * 3 + 2] = *cv;
        meshlet.triangleCount++;
	}

	if (meshlet.triangleCount > 0)
	    mesh->meshlets.push_back(meshlet);
}


static inline Mesh loadMesh(const char* path)
{
	Mesh mesh = loadObj(path);
	buildMeshlets(&mesh);
	return mesh;
}
