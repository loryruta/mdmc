#include "ai_scene_loader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// ================================================================================================================================
// Mesh
// ================================================================================================================================

void load_position_vbo(const std::shared_ptr<Mesh>& mesh, const aiMesh* ai_mesh)
{
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo[Mesh::Attribute::POSITION]);

	glBufferData(GL_ARRAY_BUFFER, size_t(ai_mesh->mNumVertices) * 3 * sizeof(GLfloat), ai_mesh->mVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(Mesh::Attribute::POSITION);
	glVertexAttribPointer(Mesh::Attribute::POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind
	glBindVertexArray(0);
}

void load_normal_vbo(const std::shared_ptr<Mesh>& mesh, const aiMesh* ai_mesh)
{
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo[Mesh::Attribute::NORMAL]);

	glEnableVertexAttribArray(Mesh::Attribute::NORMAL);

	if (ai_mesh->HasNormals())
	{
		glBufferData(GL_ARRAY_BUFFER, size_t(ai_mesh->mNumVertices) * 3 * sizeof(GLfloat), ai_mesh->mNormals, GL_STATIC_DRAW);

		glVertexAttribPointer(Mesh::Attribute::NORMAL, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	}
	else
	{
		GLfloat normals[] = { 0.0f, 0.0f, 0.0f };
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), normals, GL_STATIC_DRAW);

		glVertexAttribPointer(Mesh::Attribute::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(Mesh::Attribute::NORMAL, ai_mesh->mNumVertices);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void load_uv_vbo(const std::shared_ptr<Mesh>& mesh, const aiMesh* ai_mesh)
{
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo[Mesh::Attribute::UV]);

	glEnableVertexAttribArray(Mesh::Attribute::UV);

	if (ai_mesh->HasTextureCoords(0))
	{
		glBufferData(GL_ARRAY_BUFFER, size_t(ai_mesh->mNumVertices) * 3 * sizeof(GLfloat), ai_mesh->mTextureCoords[0], GL_STATIC_DRAW);

		glVertexAttribPointer(Mesh::Attribute::UV, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	}
	else
	{
		GLfloat uv[] = { 0.0f, 0.0f };
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat), uv, GL_STATIC_DRAW);

		glVertexAttribPointer(Mesh::Attribute::UV, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(Mesh::Attribute::UV, ai_mesh->mNumVertices);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void load_color_vbo(const std::shared_ptr<Mesh>& mesh, const aiMesh* ai_mesh)
{
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo[Mesh::Attribute::COLOR]);

	glEnableVertexAttribArray(Mesh::Attribute::COLOR);

	if (ai_mesh->HasVertexColors(Mesh::Attribute::COLOR))
	{
		glBufferData(GL_ARRAY_BUFFER, size_t(ai_mesh->mNumVertices) * 4 * sizeof(float), ai_mesh->mColors[0], GL_STATIC_DRAW);

		glVertexAttribPointer(Mesh::Attribute::COLOR, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	}
	else
	{
		GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), color, GL_STATIC_DRAW);

		glVertexAttribPointer(Mesh::Attribute::COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(Mesh::Attribute::COLOR, ai_mesh->mNumVertices);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void load_ebo(const std::shared_ptr<Mesh>& mesh, const aiMesh* ai_mesh)
{
	mesh->elements_count = size_t(ai_mesh->mNumFaces) * 3;
	GLuint* indices = new GLuint[mesh->elements_count];

	for (size_t i = 0; i < ai_mesh->mNumFaces; i++)
	{
		aiFace& face = ai_mesh->mFaces[i];

		if (face.mNumIndices != 3)
			throw; // std::cerr << "face.mNumIndices != 3" << std::endl;

		indices[i * 3] = face.mIndices[0];
		indices[i * 3 + 1] = face.mIndices[1];
		indices[i * 3 + 2] = face.mIndices[2];
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->elements_count * sizeof(GLuint), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

std::shared_ptr<Mesh> load_mesh(const aiMesh* ai_mesh, const aiMatrix4x4& ai_transform)
{
	auto mesh = std::make_shared<Mesh>();

	load_position_vbo(mesh, ai_mesh);
	load_normal_vbo(mesh, ai_mesh);
	load_color_vbo(mesh, ai_mesh);
	load_uv_vbo(mesh, ai_mesh);

	load_ebo(mesh, ai_mesh);

	mesh->transform = glm::transpose(glm::make_mat4(ai_transform[0]));

	return mesh;
}

// ================================================================================================================================
// Material
// ================================================================================================================================

void load_material_texture(const GLuint& texture, aiTextureType texture_type, const std::filesystem::path& folder, const aiMaterial* ai_material)
{
	glBindTexture(GL_TEXTURE_2D, texture);

	aiString path;
	if (aiGetMaterialTexture(ai_material, texture_type, 0, &path) == aiReturn_SUCCESS && path.length > 0)
	{
		int width, height;
		uint8_t* image_data = stbi_load((folder / path.C_Str()).u8string().c_str(), &width, &height, NULL, STBI_rgb_alpha);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

		stbi_image_free(image_data);
	}
	else
	{
		GLfloat empty_image[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, empty_image);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void load_material_color(glm::vec4& color, const char* key, unsigned int type, unsigned int index, const aiMaterial* ai_material)
{
	if (aiGetMaterialColor(ai_material, key, type, index, (aiColor4D*) &color[0]) != aiReturn_SUCCESS)
		color = glm::vec4(0);
}

std::shared_ptr<Material> load_material(const std::filesystem::path& folder, const aiMaterial* ai_material)
{
	auto material = std::make_shared<Material>();
	Material::Type type;
	
	type = Material::Type::NONE;
	load_material_texture(material->get_texture(type), aiTextureType_NONE, folder, ai_material);
	material->get_color(type) = glm::vec4(1);

	type = Material::Type::DIFFUSE;
	load_material_texture(material->get_texture(type), aiTextureType_DIFFUSE, folder, ai_material);
	load_material_color(material->get_color(type), AI_MATKEY_COLOR_DIFFUSE, ai_material);

	type = Material::Type::AMBIENT;
	load_material_texture(material->get_texture(type), aiTextureType_AMBIENT, folder, ai_material);
	load_material_color(material->get_color(type), AI_MATKEY_COLOR_AMBIENT, ai_material);

	type = Material::Type::SPECULAR;
	load_material_texture(material->get_texture(type), aiTextureType_SPECULAR, folder, ai_material);
	load_material_color(material->get_color(type), AI_MATKEY_COLOR_SPECULAR, ai_material);

	type = Material::Type::EMISSIVE;
	load_material_texture(material->get_texture(type), aiTextureType_EMISSIVE, folder, ai_material);
	load_material_color(material->get_color(type), AI_MATKEY_COLOR_EMISSIVE, ai_material);

	return material;
}


// ================================================================================================================================
// Node
// ================================================================================================================================

void load_node(const std::shared_ptr<Scene>& scene, const aiScene* ai_scene, const std::filesystem::path& folder, aiMatrix4x4 ai_transform, const aiNode* ai_node)
{
	ai_transform *= ai_node->mTransformation;

	for (size_t i = 0; i < ai_node->mNumMeshes; i++)
	{
		auto ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];

		auto mesh = load_mesh(ai_mesh, ai_transform);
		mesh->material = load_material(folder, ai_scene->mMaterials[ai_mesh->mMaterialIndex]);

		scene->meshes.push_back(mesh);
	}

	for (size_t i = 0; i < ai_node->mNumChildren; i++)
		load_node(scene, ai_scene, folder, ai_transform, ai_node->mChildren[i]);
}

// ================================================================================================================================
// Scene
// ================================================================================================================================

std::shared_ptr<Scene> aiSceneLoader::load(const std::filesystem::path& path)
{
	Assimp::Importer importer;
	const aiScene* ai_scene = importer.ReadFile(path.u8string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
	
	auto scene = std::make_shared<Scene>();
	load_node(scene, ai_scene, path.parent_path(), aiMatrix4x4(),  ai_scene->mRootNode);

	return scene;
}
