/** @file mesh.cpp */
#include "assets/mesh.hpp"
#include "core/log.hpp"
#include <meshoptimizer.h>

/**
 * @brief Load a model from file and optionally generate LODs.
 * @param path Filesystem path to the model file.
 * @param options Bitmask controlling which vertex attributes to import.
 * @param genLODs If true, generate simplified LOD index buffers.
 *
 * @remarks Uses Assimp to import, then applies meshoptimizer steps to
 *          remap, optimize and simplify indices/vertices.
 */
Model::Model(std::filesystem::path path, uint32_t options, bool genLODs) :
	m_options(options),
	m_rootPath(std::filesystem::path(path).remove_filename())
{
	auto scene = aiImportFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	if (scene) processNode(scene->mRootNode, scene, genLODs);
	else spdlog::error("No model loaded: scene is empty. Error msg: {}", aiGetErrorString());

	aiReleaseImport(scene);
}

/**
 * @brief Recursively process an Assimp node and its meshes.
 * @param node Node to process.
 * @param scene The Assimp scene the node belongs to.
 * @param genLODs Whether LODs should be generated.
 */
void Model::processNode(aiNode* node, const aiScene* scene, bool genLODs)
{
	/** Get nodes relative transform */
	aiMatrix4x4& noteTrans = node->mTransformation;
	glm::mat4 transform;

	transform[0][0] = noteTrans.a1; transform[1][0] = noteTrans.a2; transform[2][0] = noteTrans.a3; transform[3][0] = noteTrans.a4;
	transform[0][1] = noteTrans.b1; transform[1][1] = noteTrans.b2; transform[2][1] = noteTrans.b3; transform[3][1] = noteTrans.b4;
	transform[0][2] = noteTrans.c1; transform[1][2] = noteTrans.c2; transform[2][2] = noteTrans.c3; transform[3][2] = noteTrans.c4;
	transform[0][3] = noteTrans.d1; transform[1][3] = noteTrans.d2; transform[2][3] = noteTrans.d3; transform[3][3] = noteTrans.d4;

	/** Process all the node's meshes */
	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene, transform, genLODs);
	}

	/** Process child nodes */
	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, genLODs);
	}
}

/**
 * @brief Process an individual aiMesh and convert it into the internal Mesh structure.
 * @param assimpMesh Assimp mesh pointer to convert.
 * @param scene Parent aiScene pointer for materials/textures.
 * @param relativeTransform Transform matrix from the parent node.
 * @param genLODs Whether to generate LOD index buffers.
 */
void Model::processMesh(aiMesh* assimpMesh, const aiScene* scene, const glm::mat4& relativeTransform, bool genLODs)
{
	uint32_t vertexComponents = 0;
	if (m_options & positions) vertexComponents += 3;
	if (m_options & normals) vertexComponents += 2;
	if (m_options & uvs) vertexComponents += 3;
	if (m_options & tangents) vertexComponents += 3;
	if (m_options & biTangents) vertexComponents += 3;

	Mesh mesh;
	mesh.relativeTransform = relativeTransform;
	mesh.vertices.resize(vertexComponents * assimpMesh->mNumVertices);
	mesh.positions.resize(3 * assimpMesh->mNumVertices);
	mesh.indices.resize(assimpMesh->mNumFaces * 3);

	/** Vertices */
	size_t j = 0, k = 0;
	for (size_t i = 0; i < assimpMesh->mNumVertices; i++)
	{
		mesh.positions[k] = assimpMesh->mVertices[i].x; k++;
		mesh.positions[k] = assimpMesh->mVertices[i].y; k++;
		mesh.positions[k] = assimpMesh->mVertices[i].z; k++;
		if (m_options & positions) {
			mesh.vertices[j] = assimpMesh->mVertices[i].x; j++;
			mesh.vertices[j] = assimpMesh->mVertices[i].y; j++;
			mesh.vertices[j] = assimpMesh->mVertices[i].z; j++;
		}

		if (m_options & normals) {
			mesh.vertices[j] = assimpMesh->mNormals[i].x; j++;
			mesh.vertices[j] = assimpMesh->mNormals[i].y; j++;
			mesh.vertices[j] = assimpMesh->mNormals[i].z; j++;
		}

		if (m_options & uvs) {
			mesh.vertices[j] = assimpMesh->mTextureCoords[0][i].x; j++;
			mesh.vertices[j] = assimpMesh->mTextureCoords[0][i].y; j++;
		}

		if (m_options & tangents) {
			mesh.vertices[j] = assimpMesh->mTangents[i].x; j++;
			mesh.vertices[j] = assimpMesh->mTangents[i].y; j++;
			mesh.vertices[j] = assimpMesh->mTangents[i].z; j++;
		}

		if (m_options & biTangents) {
			mesh.vertices[j] = assimpMesh->mBitangents[i].x; j++;
			mesh.vertices[j] = assimpMesh->mBitangents[i].y; j++;
			mesh.vertices[j] = assimpMesh->mBitangents[i].z; j++;
		}
	}

	j = 0;
	for (size_t i = 0; i < assimpMesh->mNumFaces; i++)
	{
		mesh.indices[j] = assimpMesh->mFaces[i].mIndices[0]; j++;
		mesh.indices[j] = assimpMesh->mFaces[i].mIndices[1]; j++;
		mesh.indices[j] = assimpMesh->mFaces[i].mIndices[2]; j++;
	}

	/** Mesh optimisation */
	const size_t vertexStride = sizeof(float) * vertexComponents;
	const size_t vertexCountOnLoad = mesh.vertices.size() / vertexComponents;
	const size_t indexCountOnLoad = mesh.indices.size();

	std::vector<uint32_t> remap(mesh.indices.size());

	const size_t vertexCount = meshopt_generateVertexRemap(
		remap.data(),
		mesh.indices.data(),
		indexCountOnLoad,
		mesh.vertices.data(),
		vertexCountOnLoad,
		vertexStride
	);

	spdlog::info("vertex count {} index count {}", mesh.vertices.size(), mesh.indices.size());

	std::vector<uint32_t> remappedIndices(mesh.indices.size());
	std::vector<float> remappedVertices(vertexCount * vertexComponents);
	{
		meshopt_remapIndexBuffer(
			remappedIndices.data(),
			mesh.indices.data(),
			indexCountOnLoad,
			remap.data());


		meshopt_remapVertexBuffer(
			remappedVertices.data(),
			mesh.vertices.data(),
			vertexCountOnLoad,
			vertexStride,
			remap.data()
		);

		mesh.indices = remappedIndices;
		mesh.vertices = remappedVertices;

		spdlog::info("Verts on load: {} indices on load: {} ", vertexCountOnLoad, indexCountOnLoad);
		spdlog::info("Optimised verts: {} indices: {}", remappedVertices.size() / vertexComponents, remappedIndices.size());

		meshopt_optimizeVertexCache(
			remappedIndices.data(),
			remappedIndices.data(),
			indexCountOnLoad,
			vertexCount
		);

		meshopt_optimizeOverdraw(
			remappedIndices.data(),
			remappedIndices.data(),
			indexCountOnLoad,
			remappedVertices.data(),
			vertexCount,
			vertexStride,
			1.05f); //!< Docs suggest 1.05 is generally a good threshold

		meshopt_optimizeVertexFetch(
			remappedVertices.data(),
			remappedIndices.data(),
			indexCountOnLoad,
			remappedVertices.data(),
			vertexCount,
			vertexStride
		);

		const float threshold1 = 0.5f;	//!< LOD1 threshold, the lower the number the fewer vertices
		const float threshold2 = 0.25f; //!< LOD2 threshold
		size_t LOD1_target_index_count = size_t(remappedIndices.size() * threshold1); //!< Ideal LOD1 index count
		size_t LOD2_target_index_count = size_t(remappedIndices.size() * threshold2); //!< Ideal LOD2 index count
		const float LOD1_target_error = 0.01f; //!< Error allowed, the higher the number greater deviation allowed
		const float LOD2_target_error = 0.02f; //!< Error allowed, the higher the number greater deviation allowed

		mesh.m_indicesLOD1.resize(remappedIndices.size());

		uint32_t LOD1IndexCount = meshopt_simplify(
			mesh.m_indicesLOD1.data(),
			remappedIndices.data(),
			remappedIndices.size(),
			remappedVertices.data(),
			vertexCount,
			vertexStride,
			LOD1_target_index_count,
			LOD1_target_error
		);

		mesh.m_indicesLOD2.resize(remappedIndices.size());

		uint32_t LOD2IndexCount = meshopt_simplify(
			mesh.m_indicesLOD2.data(),
			remappedIndices.data(),
			remappedIndices.size(),
			remappedVertices.data(),
			vertexCount,
			vertexStride,
			LOD2_target_index_count,
			LOD2_target_error
		);

		mesh.indices = remappedIndices;
		mesh.vertices = remappedVertices;
		mesh.m_indicesLOD1.erase(mesh.m_indicesLOD1.begin() + LOD1IndexCount, mesh.m_indicesLOD1.end()); //!< Cut down to actual size
		mesh.m_indicesLOD2.erase(mesh.m_indicesLOD2.begin() + LOD2IndexCount, mesh.m_indicesLOD2.end()); //!< Cut down to actual size

		spdlog::info("Verts on load: {} indices on load: {} ", vertexCountOnLoad, indexCountOnLoad);
		spdlog::info("Optimised verts: {} indices: {}", remappedVertices.size() / vertexComponents, remappedIndices.size());
		spdlog::info("LOD1 target index count {}  actual index count {}", LOD1_target_index_count, LOD1IndexCount);
		spdlog::info("LOD2 target index count {}  actual index count {}", LOD2_target_index_count, LOD2IndexCount);
	}

	mesh.allIndices.insert(mesh.allIndices.end(), mesh.indices.begin(), mesh.indices.end());
	mesh.allIndices.insert(mesh.allIndices.end(), mesh.m_indicesLOD1.begin(), mesh.m_indicesLOD1.end());
	mesh.allIndices.insert(mesh.allIndices.end(), mesh.m_indicesLOD2.begin(), mesh.m_indicesLOD2.end());

	/** Set LOD data */
	mesh.m_LOD_data[0] = std::pair<uint32_t, uint32_t>(0, mesh.indices.size());
	mesh.m_LOD_data[1] = std::pair<uint32_t, uint32_t>(mesh.indices.size(), mesh.m_indicesLOD1.size());
	mesh.m_LOD_data[2] = std::pair<uint32_t, uint32_t>(mesh.indices.size() + mesh.m_indicesLOD1.size(), mesh.m_indicesLOD2.size());
	
	if (genLODs) {
		for (size_t i = 0; i < mesh.indices.size(); i++) {
			mesh.indices[i] = mesh.allIndices[i];
		}
	}
	else {
		for (size_t i = 0; i < mesh.indices.size(); i++) {
			mesh.indices[i] = mesh.indices[i];
		}
	}

	/** Texture paths */
	aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];

	for (auto type : m_textureTypes)
	{
		std::filesystem::path textureFilePath = m_rootPath;
		for (uint32_t i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString str;
			material->GetTexture(type, i, &str);
			std::filesystem::path fileName(str.C_Str());
			textureFilePath /= fileName;
			mesh.texturePaths[type] = textureFilePath;
		}
	}

	m_meshes.push_back(mesh);
}