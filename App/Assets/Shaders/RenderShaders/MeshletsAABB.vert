#version 450
#extension GL_NV_gpu_shader5 : enable

struct AABB {
    double aabbPoints[6];
};

layout(std430, binding = 1) restrict readonly buffer _cullingResult {
    uint16_t _aabbVisible[];
};

layout(std430, binding = 4) restrict readonly buffer _meshletsAABBs {
    AABB _aabbs[];
};

vec4[8] computeCorners(AABB box, mat4 matrix){
//    vec4 vmin = mModelViewProj * vec4(box.aabbPoints[1], box.aabbPoints[2], box.aabbPoints[3], 1);
//    vmin.xyz/=vmin.w;
//    vec4 vmax = mModelViewProj * vec4(box.aabbPoints[4], box.aabbPoints[5], box.aabbPoints[6], 1);
//    vmax.xyz/=vmax.w;
    vec3 vmin = vec3(box.aabbPoints[0], box.aabbPoints[1], box.aabbPoints[2]);
    vec3 vmax = vec3(box.aabbPoints[3], box.aabbPoints[4], box.aabbPoints[5]);
    vec4 corners[8] = vec4[](
        matrix * vec4(vmin.x, vmin.y, vmin.z, 1.0),
        matrix * vec4(vmin.x, vmax.y, vmin.z, 1.0),
        matrix * vec4(vmin.x, vmin.y, vmax.z, 1.0),
        matrix * vec4(vmin.x, vmax.y, vmax.z, 1.0),
        matrix * vec4(vmax.x, vmin.y, vmin.z, 1.0),
        matrix * vec4(vmax.x, vmax.y, vmin.z, 1.0),
        matrix * vec4(vmax.x, vmin.y, vmax.z, 1.0),
        matrix * vec4(vmax.x, vmax.y, vmax.z, 1.0)
    );
    return corners;
}

AABB recomputeAABB(AABB aabb, mat4 modelMatrix){
    vec4 corners[8] = computeCorners(aabb, modelMatrix);

    vec3 vmin3 = corners[0].xyz;
    vec3 vmax3 = corners[0].xyz;

    for (int i = 1; i < 8; ++i) {
        vmin3 = min(vmin3, corners[i].xyz);
        vmax3 = max(vmax3, corners[i].xyz);
    }

    AABB result;
    result.aabbPoints[0] = vmin3.x;
    result.aabbPoints[1] = vmin3.y;
    result.aabbPoints[2] = vmin3.z;
    result.aabbPoints[3] = vmax3.x;
    result.aabbPoints[4] = vmax3.y;
    result.aabbPoints[5] = vmax3.z;

    return result;
}

vec4[24] computeLineVertices(vec4[8] corners){
    vec4 vertices[24] = vec4[](
        corners[0], corners[1],
        corners[2], corners[3],
        corners[4], corners[5],
        corners[6], corners[7],

        corners[0], corners[2],
        corners[1], corners[3],
        corners[4], corners[6],
        corners[5], corners[7],

        corners[0], corners[4],
        corners[1], corners[5],
        corners[2], corners[6],
        corners[3], corners[7]
    );

    return vertices;
}

uniform mat4 viewProjMatrix;
uniform mat4 modelMatrix;
uniform uint meshletSize;
out vec3 color;

void main() {
    uint meshletIndex = gl_VertexID/24;
    AABB box = recomputeAABB(_aabbs[meshletIndex], modelMatrix);
    vec4 corners[8] = computeCorners(box, viewProjMatrix);
    vec4 vertices[24] = computeLineVertices(corners);
    gl_Position = vertices[gl_VertexID%24];
    float lod = ((float)_aabbVisible[meshletIndex])/meshletSize;
    color = mix(vec3(1.f,0,0), vec3(0,1.f,0), lod);
}