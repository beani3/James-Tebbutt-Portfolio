/**
 * @file UBO.cpp
 * @brief Wrapper around OpenGL Uniform Buffer Objects (UBOs).
 *
 * @details Provides creation, destruction and upload helpers for uniform buffer objects
 * based on a provided UBOLayout.
 */
#include "buffers/UBO.hpp"

UBO::UBO(const UBOLayout& layout) : m_layout(layout)
{
	for (auto& elem : m_layout)
	{
		m_uniformCache[elem.m_name] = std::pair<uint32_t, uint32_t>(elem.m_offset, elem.m_size);
	}

	glCreateBuffers(1, &m_ID);
	glNamedBufferStorage(m_ID, layout.getSize(), NULL, GL_DYNAMIC_STORAGE_BIT);

	glBindBufferRange(GL_UNIFORM_BUFFER, m_layout.getBindingPoint(), m_ID, 0, m_layout.getSize());
}

/**
 * @brief Destructor - deletes the OpenGL buffer object.
 */
UBO::~UBO()
{
	glDeleteBuffers(1, &m_ID);
}

/**
 * @brief Upload a single uniform value into the UBO using the uniform's name.
 * @param uniformName The name of the uniform (as specified in the UBO layout).
 * @param data Pointer to the data to upload. Must point to at least the size
 * associated with the named uniform in the layout.
 * @return true if the upload succeeded (uniform name found), false otherwise.
 *
 * @remarks Uses glNamedBufferSubData to copy the data at the precomputed
 * offset for the named uniform.
 */
bool UBO::uploadData(const std::string& uniformName, void* data)
{
	bool result = false;
	auto it = m_uniformCache.find(uniformName);
	if (it != m_uniformCache.end()) {
		auto& [offset, size] = it->second;
		glNamedBufferSubData(m_ID, offset, size, data);
		result = true;
	}
	return result;
}

/**
 * @brief Upload an entire buffer to the UBO.
 * @param data Byte vector containing the data to upload.
 * @return true if the data fits within the UBO's size and upload occurred,
 * false if the supplied data is larger than the UBO.
 */
bool UBO::uploadBuffer(const std::vector<unsigned char>& data)
{
	if (data.size() <= m_layout.getSize()) {
		glNamedBufferSubData(m_ID, 0, data.size(), (void*)data.data());
		return true;
	}
	return false;
}