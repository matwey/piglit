// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable
#extension GL_ARB_arrays_of_arrays: enable

// From Section 4.4.6.2 (Format Layout Qualifiers) of the GLSL 4.50 spec:
//
// "It is a compile-time error to declare an image variable where the format
//  qualifier does not match the image variable type."

struct S_inner {
	layout (r32i) image2D img;
};

struct {
	S_inner si[2][2];
} s;

void main()
{
}
