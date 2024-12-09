#version 450
layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;

uniform mat4 mModelViewProj;

vec2 vertices[6] = vec2[](
    vec2(-1, -1),
     vec2(1, -1),
     vec2(1, 1),
     vec2(1, 1),
     vec2(-1, 1),
     vec2(-1, -1)
);

vec3 UnprojectPoint(float x, float y, float z) {
    mat4 inv = inverse(mModelViewProj);
    vec4 unprojectedPoint =  inv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main (){
    nearPoint = UnprojectPoint(vertices[gl_VertexID].x, vertices[gl_VertexID].y, 0.0).xyz;
    farPoint = UnprojectPoint(vertices[gl_VertexID].x, vertices[gl_VertexID].y, 1.0).xyz;
	gl_Position = vec4(vertices[gl_VertexID], 0, 1.0);
}