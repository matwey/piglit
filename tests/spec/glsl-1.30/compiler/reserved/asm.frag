// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Check that 'asm' is a reserved keyword.

#version 130

int f()
{
	int asm;
	return 0;
}
