#version 450
#extension GL_NV_gpu_shader5 : enable


layout (std430, binding = 0) restrict readonly buffer vPosition{
	float points[];
};

layout (std430, binding = 1) restrict readonly buffer vColor{
	uint rgbColor[];
};

layout (std430, binding = 2) restrict readonly buffer vNormal{
	vec3 normals[];
};
layout (std430, binding = 3) restrict readonly buffer vFusionData{
	float fusionData[];
};

layout (std430, binding = 4) restrict readonly buffer vNdVegetationIndex{
	float NdVegetationIndex[];
};
layout (std430, binding = 5) restrict readonly buffer vRVegetationIndex{
	float RVegetationIndex[];
};
layout (std430, binding = 6) restrict readonly buffer vGrVegetationIndex{
	float GrVegetationIndex[];
};
layout (std430, binding = 7) restrict readonly buffer vNdreVegetationIndex{
	float NdreVegetationIndex[];
};

layout (std430, binding = 9) restrict readonly buffer _cullingResult{
	uint16_t _aabbVisible[];
};


uniform uint meshletSize;
uniform mat4 modelMatrix;
uniform mat4 mModelViewProj;
uniform vec4 classColor;
uniform vec3 cameraPosition;
uniform vec2 ndVIThreshold;
uniform vec2 rVIThreshold;
uniform vec2 grVIThreshold;
uniform vec2 ndreVIThreshold;
uniform vec4 fusionGradient[256];

subroutine vec4 VertexColorSource(float dist);
subroutine uniform VertexColorSource VertexColorSourceUniform;

subroutine vec4 FilterBasedOnValue(float dist);
subroutine uniform FilterBasedOnValue FilterBasedOnValueUniform;

subroutine(VertexColorSource)
vec4 useClassColor(float dist){
	return classColor;
}

subroutine(VertexColorSource)
vec4 usePointColor(float dist){
	vec4 color = unpackUnorm4x8(rgbColor[gl_VertexID]);
	color[3]=1;
	return color;
}

subroutine(VertexColorSource)
vec4 useFusionDataAsColor(float dist){
	uint gradientIdx = (uint)(fusionData[gl_VertexID] * 256);
	if (gradientIdx > 0){
		return vec4(fusionGradient[gradientIdx].rgb, 1);
	}else{
		return vec4(0, 0, 0, 1);
	}
}

subroutine(VertexColorSource)
vec4 useNormalAsColor(float dist){
	return vec4(normals[gl_VertexID], 1);
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

subroutine(VertexColorSource)
vec4 useMeshletColor(float dist){
	uint meshletIndex = gl_VertexID / meshletSize;
	return vec4(rand(vec2(meshletIndex, 1)), rand(vec2(meshletIndex, 2)), rand(vec2(meshletIndex, 3)), 1);
}

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

bool isInRange(float value, vec2 range){
	return range[0]<value && value<range[1];
}

subroutine(FilterBasedOnValue)
vec4 dontUseFilter(float dist){
	return VertexColorSourceUniform(dist);
}

subroutine(FilterBasedOnValue)
vec4 vegetationFilter(float dist){
	if(isInRange(NdVegetationIndex[gl_VertexID], ndVIThreshold) && isInRange(RVegetationIndex[gl_VertexID], rVIThreshold) && 
	isInRange(GrVegetationIndex[gl_VertexID], grVIThreshold)&& isInRange(NdreVegetationIndex[gl_VertexID], ndreVIThreshold)){
		return VertexColorSourceUniform(dist);
	}
	return vec4(0,0,0,0);
}

out vec4 finalColor;

void main() {
	uint meshletIndex = gl_VertexID/meshletSize;
    if( (uint16_t)_aabbVisible[meshletIndex] > (uint16_t)0){
        uint baseIndex = gl_VertexID*3;
        vec4 position = vec4(points[baseIndex], points[baseIndex+1], points[baseIndex+2], 1);	
		vec4 projectedPosition = mModelViewProj * position;
		float dist = projectedPosition.z;
		float size = 150/dist;
		float pointSize = min(max(size, 1), 150);
		gl_Position = projectedPosition;
		gl_PointSize = pointSize;
		vec4 pointColor = FilterBasedOnValueUniform(dist);
		finalColor = pointColor;
	}else{
		gl_Position = vec4(2,2,2,1); //Se pone fuera de rango [-1, 1] para que no se dibuje
	}
}