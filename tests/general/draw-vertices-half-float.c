/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Marek Olšák <maraeo@gmail.com>
 */

/* The test for some tricky bits of the OpenGL vertex submission.
 * The emphasis is taken on non-dword-aligned strides and offsets.
 * This one is for half float vertices.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_es_version = 20;

	config.window_width = 320;
	config.window_height = 60;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

GLboolean user_va = GL_FALSE;

static GLenum vertex_array_type;

#ifndef PIGLIT_USE_OPENGL
static const char *vs_text =
    "attribute vec4 position;\n"
    "uniform mat4 projection;\n"
    "void main() { gl_Position = projection * position; }\n";

static const char *fs_text =
    "precision mediump float;\n"
    "void main() { gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); }\n";

static GLuint prog;
static GLint projection_loc;
static GLint position_loc;
#endif

void piglit_init(int argc, char **argv)
{
    unsigned i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "user")) {
            user_va = GL_TRUE;
            puts("Testing user vertex arrays.");
        }
    }

#ifdef PIGLIT_USE_OPENGL
    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    piglit_require_gl_version(15);
    piglit_require_extension("GL_ARB_half_float_vertex");

    glShadeModel(GL_FLAT);

    vertex_array_type = GL_HALF_FLOAT_ARB;
#else
    piglit_require_extension("GL_OES_vertex_half_float");

    prog = piglit_build_simple_program(vs_text, fs_text);
    glUseProgram(prog);
    position_loc = glGetAttribLocation(prog, "position");
    projection_loc = glGetUniformLocation(prog, "projection");

    piglit_ortho_uniform(projection_loc, piglit_width, piglit_height);

    vertex_array_type = GL_HALF_FLOAT_OES;
#endif
    glClearColor(0.2, 0.2, 0.2, 1.0);
}

static GLuint vboVertexPointer(GLint size, GLenum type, GLsizei stride,
                               const GLvoid *buf, GLsizei bufSize, intptr_t bufOffset)
{
    GLuint id;
    if (user_va) {
#ifdef PIGLIT_USE_OPENGL
        glVertexPointer(size, type, stride, (char*)buf + bufOffset);
#else
        glEnableVertexAttribArray(position_loc);
        glVertexAttribPointer(position_loc, size, type, GL_FALSE, stride, (char*)buf + bufOffset);
#endif
        return 0;
    }
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
#ifdef PIGLIT_USE_OPENGL
    glVertexPointer(size, type, stride, (void*)bufOffset);
#else
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(position_loc, size, type, GL_FALSE, stride, (void*)bufOffset);
#endif
    return id;
}

static void test_half_vertices_wrapped(unsigned short x1, unsigned short y1,
                                       unsigned short x2, unsigned short y2,
                                       unsigned short one, int index)
{
    unsigned short v2[] = {
        x1, y1,
        x1, y2,
        x2, y1
    };
    unsigned short v3[] = {
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    unsigned short v4[] = {
        x1, y1, 0, one,
        x1, y2, 0, one,
        x2, y1, 0, one
    };
    unsigned short v2o[] = {
        0,
        x1, y1,
        x1, y2,
        x2, y1
    };
    unsigned short v3o[] = {
        0,
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    unsigned short v4o[] = {
        0,
        x1, y1, 0, one,
        x1, y2, 0, one,
        x2, y1, 0, one
    };
    GLuint vbo;

    switch (index) {
        case 0: vbo = vboVertexPointer(2, vertex_array_type, 4, v2, sizeof(v2), 0); break;

        case 1: vbo = vboVertexPointer(2, vertex_array_type, 6, v3, sizeof(v3), 0); break;
        case 2: vbo = vboVertexPointer(3, vertex_array_type, 6, v3, sizeof(v3), 0); break;

        case 3: vbo = vboVertexPointer(2, vertex_array_type, 8, v4, sizeof(v4), 0); break;
        case 4: vbo = vboVertexPointer(3, vertex_array_type, 8, v4, sizeof(v4), 0); break;
        case 5: vbo = vboVertexPointer(4, vertex_array_type, 8, v4, sizeof(v4), 0); break;

        case 6: vbo = vboVertexPointer(2, vertex_array_type, 4, v2o, sizeof(v2o), 2); break;

        case 7: vbo = vboVertexPointer(2, vertex_array_type, 6, v3o, sizeof(v3o), 2); break;
        case 8: vbo = vboVertexPointer(3, vertex_array_type, 6, v3o, sizeof(v3o), 2); break;

        case 9: vbo = vboVertexPointer(2, vertex_array_type, 8, v4o, sizeof(v4o), 2); break;
        case 10:vbo = vboVertexPointer(3, vertex_array_type, 8, v4o, sizeof(v4o), 2); break;
        case 11:vbo = vboVertexPointer(4, vertex_array_type, 8, v4o, sizeof(v4o), 2); break;

        default:vbo = 0; assert(0); break;
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (vbo)
        glDeleteBuffers(1, &vbo);
}

static void test_half_vertices(float fx1, float fy1, float fx2, float fy2, int index)
{
    unsigned short x1, y1, x2, y2, one;
    x1 = piglit_half_from_float(fx1);
    y1 = piglit_half_from_float(fy1);
    x2 = piglit_half_from_float(fx2);
    y2 = piglit_half_from_float(fy2);
    one = piglit_half_from_float(1);

    test_half_vertices_wrapped(x1, y1, x2, y2, one, index);
}

struct test {
    void (*test)(float x1, float y1, float x2, float y2, int index);
    int index;
    float expected_color[4];
    const char *name;
};

struct test tests[] = {
    {test_half_vertices, 0, {1, 1, 1, 1}, "Half vertices - components: 2, stride: 4, offset: 0"},
    {test_half_vertices, 1, {1, 1, 1, 1}, "Half vertices - components: 2, stride: 6, offset: 0"},
    {test_half_vertices, 2, {1, 1, 1, 1}, "Half vertices - components: 3, stride: 6, offset: 0"},
    {test_half_vertices, 3, {1, 1, 1, 1}, "Half vertices - components: 2, stride: 8, offset: 0"},
    {test_half_vertices, 4, {1, 1, 1, 1}, "Half vertices - components: 3, stride: 8, offset: 0"},
    {test_half_vertices, 5, {1, 1, 1, 1}, "Half vertices - components: 4, stride: 8, offset: 0"},
    {test_half_vertices, 6, {1, 1, 1, 1}, "Half vertices - components: 2, stride: 4, offset: 2"},
    {test_half_vertices, 7, {1, 1, 1, 1}, "Half vertices - components: 2, stride: 6, offset: 2"},
    {test_half_vertices, 8, {1, 1, 1, 1}, "Half vertices - components: 3, stride: 6, offset: 2"},
    {test_half_vertices, 9, {1, 1, 1, 1}, "Half vertices - components: 2, stride: 8, offset: 2"},
    {test_half_vertices, 10, {1, 1, 1, 1}, "Half vertices - components: 3, stride: 8, offset: 2"},
    {test_half_vertices, 11, {1, 1, 1, 1}, "Half vertices - components: 4, stride: 8, offset: 2"},

    {0}
};

enum piglit_result
piglit_display(void)
{
    GLboolean pass = GL_TRUE;
    unsigned i;
    float x = 0, y = 0;

    glClear(GL_COLOR_BUFFER_BIT);
#ifdef PIGLIT_USE_OPENGL
    glEnableClientState(GL_VERTEX_ARRAY);
#endif

    for (i = 0; tests[i].test; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        printf("%s\n", tests[i].name);
        tests[i].test(x, y, x+20, y+20, tests[i].index);
        if (!piglit_check_gl_error(GL_NO_ERROR))
		 piglit_report_result(PIGLIT_FAIL);
        pass = piglit_probe_pixel_rgba(x+5, y+5, tests[i].expected_color) && pass;

        x += 20;
        if (x > 300) {
            x = 0;
            y += 20;
        }
    }

    glFinish();
    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

