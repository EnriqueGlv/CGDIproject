#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <imgui.h>
// #include "imgui_impl_glut.h"
// #include "imgui_impl_opengl2.h"
// #define GL_SILENCE_DEPRECATION
// #ifdef __APPLE__
// #include <GLUT/glut.h>
// #else
// #include <GL/freeglut.h>
// #endif
// #include "imgui.h"

#include "utils.cpp"

int main(){
	// Load image in OpenGL
	int my_image_width = 0;
	int my_image_height = 0;
	GLuint my_image_texture = 0;
	bool ret = LoadTextureFromFile("../images/castle_orig.jpg", &my_image_texture, &my_image_width, &my_image_height);
	IM_ASSERT(ret);

	// Open image in ImGUI
	ImGui::Begin("OpenGL Texture Text");
	ImGui::Text("pointer = %p", my_image_texture);
	ImGui::Text("size = %d x %d", my_image_width, my_image_height);
	ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
	ImGui::End();
}