#version 450
layout(location = 1) in vec3 nearPoint;
layout(location = 2) in vec3 farPoint;
layout(location = 0) out vec4 outColor;

float near = 0.01f;
float far = 100.0f;

uniform mat4 mModelViewProj;
uniform vec3 gridColor;

vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(gridColor.xyz, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.xyz = vec3(0,0,1);
    // x axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.xyz = vec3(1,0,0);
    return color;
}
float computeDepth(vec3 pos) {
    vec4 clip_space_pos = mModelViewProj * vec4(pos, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}
float computeLinearDepth(float depth) {
    float clip_space_depth = depth * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100
    return linearDepth / far; // normalize
}
void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    float depth = computeDepth(fragPos3D);
    gl_FragDepth = depth;

    float linearDepth = computeLinearDepth(depth);
    float fading = max(0, (0.15 - linearDepth));

    outColor = (grid(fragPos3D, 10) + grid(fragPos3D, 1))* float(t > 0); // adding multiple resolution for the grid
    outColor.a *= fading;
}