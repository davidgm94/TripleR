#include <indexgenerator.h>
#include <fast_obj.h>
typedef struct
{
	float vx, vy, vz;
	u8 nx, ny, nz, nw;
	float tu, tv;
} Vertex;

FAST_VECTOR(Vertex);

typedef struct
{
	fast_vector_Vertex vertices;
	fast_vector_u32 indices;
} Mesh;

typedef struct
{
	u32 vertices[64];
	u8 indices[126]; // up to 42 triangles
	u8 indexCount;
	u8 vertexCount;
} Meshlet;
FAST_VECTOR(Meshlet);

typedef struct
{
	fast_vector_Vertex vertices;
	fast_vector_u32 indices;
	fast_vector_Meshlet meshlets;
} Mesh_with_meshlets;

static inline Mesh_with_meshlets loadObj(const char* path)
{
	fastObjMesh* obj = fast_obj_read(path);

	size_t indexCount = 0;

	for (u32 i = 0; i < obj->face_count; i++)
	{
		indexCount += 3 * (obj->face_vertices[i] - 2);
	}

	fast_vector_Vertex vertices;
	allocate_fast_vector_Vertex(&vertices, indexCount);

	u64 vertexOffset = 0;
	u64 indexOffset = 0;

	for (u32 i = 0; i < obj->face_count; i++)
	{
		for (u32 j = 0; j < obj->face_vertices[i]; j++)
		{
			fastObjIndex gi = obj->indices[indexOffset + j];
			if (j >= 3)
			{
				vertices.data[vertexOffset + 0] = vertices.data[vertexOffset - 3];
				vertices.data[vertexOffset + 1] = vertices.data[vertexOffset - 1];
				vertexOffset += 2;
			}

			Vertex v;
			v.vx = obj->positions[gi.p * 3 + 0];
			v.vy = obj->positions[gi.p * 3 + 1];
			v.vz = obj->positions[gi.p * 3 + 2];

			v.nx = (u8)(obj->normals[gi.n * 3 + 0] * 127.f + 127.5f);
			v.ny = (u8)(obj->normals[gi.n * 3 + 1] * 127.f + 127.5f);
			v.nz = (u8)(obj->normals[gi.n * 3 + 2] * 127.f + 127.5f);
			
			v.tu = obj->texcoords[gi.t * 2 + 0];
			v.tv = obj->texcoords[gi.t * 2 + 1]; // originally: obj->texcoords[gi.t * 2 + 1]

			vertices.data[vertexOffset] = v;
			vertexOffset++;
		}

		indexOffset += obj->face_vertices[i];
	}

	os_assert(vertexOffset == indexCount);
	fast_obj_destroy(obj);

	Mesh_with_meshlets mesh;
	// TODO: This procedure should be extracted into another function
	if (false)
	{
		mesh.vertices = vertices;
		allocate_fast_vector_u32(&mesh.indices, vertices.size);

		for (u32 i = 0; i < indexCount; i++)
		{
			mesh.indices.data[i] = (u32)(i);
		}
	}
	else
	{
		fast_vector_u32 remap;
		allocate_fast_vector_u32(&remap, indexCount);
		u64 vertexCount = meshopt_generateVertexRemap(remap.data, null, indexCount, vertices.data, indexCount, sizeof(Vertex));

		allocate_fast_vector_Vertex(&mesh.vertices, vertexCount);
		allocate_fast_vector_u32(&mesh.indices, indexCount);
		
		meshopt_remapVertexBuffer(mesh.vertices.data, vertices.data, indexCount, sizeof(Vertex), remap.data);
		meshopt_remapIndexBuffer(mesh.indices.data, null, indexCount, remap.data);

		os_free(remap.data);
		os_free(vertices.data);
	}
	mesh.meshlets.data = null;
	mesh.meshlets.size = 0;

	return mesh;
}

// TODO: fill this function by extracting the stated above
// void optimizeMesh(Mesh_with_meshlets mesh, Vertex* vertices, u64 vertexCount, u32* indices, u64 indexCount)
// {
// }

void buildMeshlets(Mesh_with_meshlets* mesh)
{
	Meshlet meshlet = {};

	fast_vector_u8 meshletVertices;
	allocate_fast_vector_u8(&meshletVertices, mesh->vertices.size);
	os_memset(meshletVertices.data, 0xff, mesh->vertices.size);

	const u64 meshIndexCount = mesh->indices.size;

	void* ptr = os_malloc(700 * 1024 * 1024);
	mesh->meshlets.data = ptr;

	for (u64 i = 0; i < meshIndexCount; i+= 3)
	{
		u64 a = mesh->indices.data[i + 0];
		u64 b = mesh->indices.data[i + 1];
		u64 c = mesh->indices.data[i + 2];

		u8* av = &meshletVertices.data[a];
		u8* bv = &meshletVertices.data[b];
		u8* cv = &meshletVertices.data[c];

		if (meshlet.vertexCount + (*av == 0xff) + (*bv == 0xff) + (*cv == 0xff) > 64 || meshlet.indexCount + 3 > 126)
		{
			os_memcpy(&mesh->meshlets.data[mesh->meshlets.size++], &meshlet, sizeof(Meshlet));

			for (u64 j = 0; j < meshlet.vertexCount; ++j)
			{
				meshletVertices.data[meshlet.vertices[j]] = 0xff;
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

		meshlet.indices[meshlet.indexCount++] = *av;
		meshlet.indices[meshlet.indexCount++] = *bv;
		meshlet.indices[meshlet.indexCount++] = *cv;
	}

	if (meshlet.indexCount > 0)
		os_memcpy(&mesh->meshlets.data[mesh->meshlets.size++], &meshlet, sizeof(Meshlet));
}

static inline Mesh_with_meshlets loadMesh(const char* path)
{
	Mesh_with_meshlets mesh = loadObj(path);
	buildMeshlets(&mesh);
	return mesh;
}