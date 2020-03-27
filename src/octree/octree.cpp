#include "octree.hpp"

Octree::Octree(size_t max_resolution) :
	max_resolution(max_resolution)
{
	size_t capacity = Octree::size(max_resolution);
	this->buffer.load_data(capacity, NULL, GL_DYNAMIC_DRAW);
	this->buffer.set_format(GL_R32UI);
}

void Octree::bind(GLuint binding)
{
	this->buffer.bind(binding, 0, GL_FALSE, NULL, GL_READ_WRITE, GL_R32UI);
}

size_t Octree::size(glm::uint level)
{
	size_t res = 0;
	do
	{
		res += pow(8, level) * 8;
		level--;
	} while (level != 0);
	return res;
}