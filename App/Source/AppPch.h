#pragma once
#include "warning_pragmas.h"

#define USE_STD_FILESYSTEM
#include <filesystem>
#include <ranges>

//https://github.com/g-truc/glm/blob/master/manual.md#section2_9
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_INLINE
#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_CXX17
#include <glm/vec2.hpp>               // vec2, bvec2, dvec2, ivec2 and uvec2
#include <glm/vec3.hpp>               // vec3, bvec3, dvec3, ivec3 and uvec3
#include <glm/vec4.hpp>               // vec4, bvec4, dvec4, ivec4 and uvec4
#include <glm/mat3x3.hpp>             // mat3, dmat3
#include <glm/mat4x4.hpp>             // mat4, dmat4
#include <glm/common.hpp>             // all the GLSL common functions: abs, min, mix, isnan, fma, etc.
#include <glm/geometric.hpp>          // all the GLSL geometry functions: dot, cross, reflect, etc.

#include <glm/ext/matrix_transform.hpp>     // translate, rotate
#include <glm/ext/matrix_clip_space.hpp>    // perspective
#include <glm/gtc/epsilon.hpp> 

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

//Logger
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h" // support for basic file logging

#include "Utilities/Typedefs.h"

// i18n
#include "locale_strings.h"

//Core
#include "DataTypes/PointCloud.h"
#include "Graphics/Renderer.h"
#include "IO/FileManager.h"

//UI
#include "UI/GUI.h"
#include "UI/GuiUtilities.h"
#include "UI/Popups/PopupSystem.h"
#include "UI/Addons/ImGuiNotify.hpp"
#include "UI/Addons/FileDialog/ImGuiFileDialog.h"
#include "Fonts/IconsFontAwesome6.h"