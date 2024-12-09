#pragma once
#include <cstdint>
#include <glm/vec2.hpp>               // vec2, bvec2, dvec2, ivec2 and uvec2
#include <glm/vec3.hpp>               // vec3, bvec3, dvec3, ivec3 and uvec3
#include <glm/vec4.hpp>               // vec4, bvec4, dvec4, ivec4 and uvec4
#include <glm/mat3x3.hpp>             // mat3, dmat3
#include <glm/mat4x4.hpp>             // mat4, dmat4
// Rename
typedef glm::vec4 vec4;
typedef glm::dvec4 dvec4;
typedef glm::mat4 mat4;
typedef glm::dmat4 dmat4;

typedef glm::vec3 vec3;
typedef glm::dvec3 dvec3;
typedef glm::mat3 mat3;
typedef glm::dmat3 dmat3;

typedef glm::vec2 vec2;
typedef glm::vec3 uvec3;
typedef glm::uvec2 uvec2;

typedef glm::ivec4 ivec4;
typedef glm::ivec3 ivec3;
typedef glm::ivec2 ivec2;


typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef double f64;
typedef float f32;