#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <vector>

//#include <gl/glew.h>
//#include <PVRTexture.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include "GLError.h"
#include <gli/gli.hpp>

class TextureLoader
{
public:

	static GLuint CreateSimpleTextureCubemap()
	{
		GLuint texture_id;
		// Six 1x1 RGB faces
		GLubyte cubePixels[6][3] =
		{
			// Face 0 - Red
			255, 0, 0,
			// Face 1 - Green,
			0, 255, 0,
			// Face 2 - Blue
			0, 0, 255,
			// Face 3 - Yellow
			255, 255, 0,
			// Face 4 - Purple
			255, 0, 255,
			// Face 5 - White
			255, 255, 255
		};

		// Generate a texture object
		glGenTextures(1, &texture_id);

		// Bind the texture object
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
		// Load the cube face - Positive X
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, 1, 1, 0,
			GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[0]);
		// Load the cube face - Negative X
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, 1, 1, 0,
			GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[1]);
		// Load the cube face - Positive Y
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, 1, 1, 0,
			GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[2]);
		// Load the cube face - Negative Y
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, 1, 1, 0,
			GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[3]);
		// Load the cube face - Positive Z
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, 1, 1, 0,
			GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[4]);
		// Load the cube face - Negative Z
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, 1, 1, 0,
			GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[5]);
		// Set the filtering mode
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		check_gl_error();
		_textures.push_back(texture_id);
		return texture_id;
	}

	static GLuint CreateTextureCubemap(const char* path)
	{
		gli::textureCube texture(gli::load_dds(path));
		assert(!texture.empty());
		//printf("%d %d\n", texture.levels(), texture.layers());
		gli::gl GL;
		gli::gl::format const format = GL.translate(texture.format());
		//printf("%s\n\t%X %X %X\n", path, format.Internal, format.External, format.Type);
		GLuint textureName = 0;
		glGenTextures(1, &textureName);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureName);
		//auto l = texture.levels();
		//auto w = texture[0].dimensions().x;
		//auto h = texture[0].dimensions().y;
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, GLint(texture.levels()),
			format.Internal,
			GLsizei(texture[0].dimensions().x),
			GLsizei(texture[0].dimensions().y));

		assert(!gli::is_compressed(texture.format()));

		for (int face = 0; face < 6; face++)
			for (int level = 0; level < texture.levels(); ++level)
			{ 
				auto t = texture[face][level];
				glTexSubImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + GLenum(face),
					level,
					0, 0,
					GLsizei(t.dimensions().x),
					GLsizei(t.dimensions().y),
					format.External,
					format.Type,
					t.data()
					);
			}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		check_gl_error();
		_textures.push_back(textureName);
		return textureName;
	}

	static GLuint CreateTexture(const char* path, bool srgb = false)
	{
		gli::texture2D texture(gli::load_dds(path));
		assert(!texture.empty());
		//printf("%d %d\n", Texture.levels(), Texture.layers());
		gli::gl GL;
		gli::gl::format const format = GL.translate(texture.format());
		//printf("%s\n\t%X %X %X\n", path, format.Internal, format.External, format.Type);
		GLuint texture_name = 0;
		glGenTextures(1, &texture_name);
		glBindTexture(GL_TEXTURE_2D, texture_name);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));

		int f = format.Internal;
		if (srgb && f == gli::gl::INTERNAL_RGBA_DXT5)
		{
			//f = 0x8C4F;
			f = gli::gl::INTERNAL_SRGB_ALPHA_DXT5;
		}
		check_gl_error();
		glTexStorage2D(GL_TEXTURE_2D, static_cast<GLint>(texture.levels()),
			f,
			static_cast<GLsizei>(texture.dimensions().x),
			static_cast<GLsizei>(texture.dimensions().y));
		check_gl_error();

		if (gli::is_compressed(texture.format()))
		{

			for (std::size_t level = 0; level < texture.levels(); ++level)
			{
				check_gl_error();
				glCompressedTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(level),
					0, 0,
					static_cast<GLsizei>(texture[level].dimensions().x),
					static_cast<GLsizei>(texture[level].dimensions().y),
					f,
					static_cast<GLsizei>(texture[level].size()),
					texture[level].data());
				check_gl_error();
			}
		}
		else
		{
			for (std::size_t Level = 0; Level < texture.levels(); ++Level)
			{
				check_gl_error();
				glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(Level),
					0, 0,
					static_cast<GLsizei>(texture[Level].dimensions().x),
					static_cast<GLsizei>(texture[Level].dimensions().y),
					format.External,
					format.Type,
					texture[Level].data());
				check_gl_error();
			}
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		check_gl_error();
		_textures.push_back(texture_name);
		return texture_name;
	}

	static GLuint CreateTexture_patch(const char* path, unsigned int internal_format, unsigned int external_format, unsigned int type)
	{
		gli::texture2D texture(gli::load_dds(path));
		assert(!texture.empty());
		//gli::texture2D texture(flip(texture_old));
		//gli::texture2D texture(texture_old);
		//printf("%d %d\n", Texture.levels(), Texture.layers());
		gli::gl GL;
		gli::gl::format const format = GL.translate(texture.format());
		printf("%s\n\t%X %X %X\n", path, format.Internal, format.External, format.Type);
		if (external_format == 0)
			external_format = format.External;
		if (type == 0)
			type = format.Type;
		GLuint texture_name = 0;
		glGenTextures(1, &texture_name);
		glBindTexture(GL_TEXTURE_2D, texture_name);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, format.Swizzle[0]);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, format.Swizzle[1]);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, format.Swizzle[2]);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, format.Swizzle[3]);
		//glTexStorage3D(GL_TEXTURE_2D, static_cast<GLint>(Texture.levels()),
		//	Format.Internal,
		//	static_cast<GLsizei>(Texture.dimensions().x),
		//	static_cast<GLsizei>(Texture.dimensions().y), 1);

		if (gli::is_compressed(texture.format()))
		{
			for (std::size_t level = 0; level < texture.levels(); ++level)
			{
				glCompressedTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level),
					internal_format,
					static_cast<GLsizei>(texture[level].dimensions().x),
					static_cast<GLsizei>(texture[level].dimensions().y),
					0,
					static_cast<GLsizei>(texture[level].size()),
					texture[level].data());
			}
		}
		else
		{
			for (std::size_t Level = 0; Level < texture.levels(); ++Level)
			{
				glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(Level),
					internal_format,
					static_cast<GLsizei>(texture[Level].dimensions().x),
					static_cast<GLsizei>(texture[Level].dimensions().y),
					static_cast<GLsizei>(0),
					external_format, type,
					texture[Level].data());

				//glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(Level),
				//	0, 0, 
				//	static_cast<GLsizei>(texture[Level].dimensions().x),
				//	static_cast<GLsizei>(texture[Level].dimensions().y),
				//	//static_cast<GLsizei>(1),
				//	format.External, format.Type,
				//	texture[Level].data());
			}
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		_textures.push_back(texture_name);
		return texture_name;
	}

	static GLuint CreateTextureArray(char const* Filename)
	{
		gli::texture2D Texture(gli::load_dds(Filename));
		assert(!Texture.empty());
		//printf("%d %d\n", Texture.levels(), Texture.layers());
		gli::gl GL;
		gli::gl::format const Format = GL.translate(Texture.format());
		GLuint texture_id = 0;
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, Format.Swizzle[0]);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, Format.Swizzle[1]);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, Format.Swizzle[2]);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, Format.Swizzle[3]);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(Texture.levels()),
			Format.Internal,
			static_cast<GLsizei>(Texture.dimensions().x),
			static_cast<GLsizei>(Texture.dimensions().y),
			static_cast<GLsizei>(1));
		if (gli::is_compressed(Texture.format()))
		{
			for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
			{
				glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(Level),
					0, 0, 0,
					static_cast<GLsizei>(Texture[Level].dimensions().x),
					static_cast<GLsizei>(Texture[Level].dimensions().y),
					static_cast<GLsizei>(1),
					Format.External,
					static_cast<GLsizei>(Texture[Level].size()),
					Texture[Level].data());
			}
		}
		else
		{
			for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
			{
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(Level),
					0, 0, 0,
					static_cast<GLsizei>(Texture[Level].dimensions().x),
					static_cast<GLsizei>(Texture[Level].dimensions().y),
					static_cast<GLsizei>(1),
					Format.External, Format.Type,
					Texture[Level].data());
			}
		}
		check_gl_error();
		_textures.push_back(texture_id);
		return texture_id;
	}

//	static GLuint print_pvr_tex_format(const char* path, unsigned int dxgi_format, bool srgb = false)
//	{
//		pvrtexture::CPVRTexture tex_test(path);
//		unsigned int i_type;
//		unsigned int e_type;
//		unsigned int type;
//		//tex_test.getOGLFormat(i_type, e_type, type);
//		//printf("%d %d %d\n", i_type, e_type, type);
//
//		tex_test.getOGLFormat(i_type, e_type, type);
//		printf("%s [pvr]\n", path);
//		printf("\t%X %X %X\n", i_type, e_type, type);
//		if (dxgi_format != 0)
//			tex_test.setDXGIFormat(dxgi_format);
//		tex_test.getOGLFormat(i_type, e_type, type);
//		printf("\t%X %X %X\n", i_type, e_type, type);
//		if (srgb)
//			tex_test.setColourSpace(ePVRTCSpacesRGB);
//		tex_test.getOGLFormat(i_type, e_type, type);
//		printf("\t%X %X %X\n", i_type, e_type, type);
//		//tex_test.~CPVRTexture();
//
//		//return CreateTexture_patch(path, i_type, e_type, type);
//		return 0;
//	}

	static GLuint CreateTexture3D(const char* path)
	{
		gli::texture3D texture(gli::load_dds(path));
		assert(!texture.empty());
		//printf("%d %d\n", texture.levels(), texture.layers());
		gli::gl GL;
		gli::gl::format const format = GL.translate(texture.format());
		//printf("%d %d %d\n", texture.dimensions().x, texture.dimensions().y, texture.dimensions().z);
		GLuint texture_id = 0;
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_3D, texture_id);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, Format.Swizzle[0]);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, Format.Swizzle[1]);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, Format.Swizzle[2]);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, Format.Swizzle[3]);
		//glTexStorage3D(GL_TEXTURE_3D, static_cast<GLint>(texture.levels()),
		//	format.Internal,
		//	static_cast<GLsizei>(texture.dimensions().x),
		//	static_cast<GLsizei>(texture.dimensions().y),
		//	static_cast<GLsizei>(texture.dimensions().z));

		if (gli::is_compressed(texture.format()))
		{
			for (std::size_t level = 0; level < texture.levels(); ++level)
			{
				glCompressedTexImage3D(GL_TEXTURE_3D, static_cast<GLint>(level),
					format.Internal,
					static_cast<GLsizei>(texture[level].dimensions().x),
					static_cast<GLsizei>(texture[level].dimensions().y),
					static_cast<GLsizei>(texture[level].dimensions().z),
					0,
					static_cast<GLsizei>(texture[level].size()),
					texture[level].data());
			}
		}
		else
		{
			for (std::size_t Level = 0; Level < texture.levels(); ++Level)
			{
				glTexImage3D(GL_TEXTURE_3D, static_cast<GLint>(Level),
					format.Internal,
					static_cast<GLsizei>(texture[Level].dimensions().x),
					static_cast<GLsizei>(texture[Level].dimensions().y),
					static_cast<GLsizei>(texture[Level].dimensions().z),
					0,
					//static_cast<GLsizei>(1),
					format.External, format.Type,
					texture[Level].data());
			}
		}

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glGenerateMipmap(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, 0);

		_textures.push_back(texture_id);
		return texture_id;
	}

	static void shut_down()
	{
		glDeleteTextures(_textures.size(), &_textures[0]);
	}

private:
	TextureLoader();

	static std::vector<GLuint> _textures;
};


#endif