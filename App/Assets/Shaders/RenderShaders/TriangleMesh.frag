#version 450

// ------------ Constraints ------------
#define CUTOFF .8f
#define PI 3.14159265359f

// ------------------------------------
// ---------- GEOMETRY ----------------
// ------------------------------------
in vec3 position;
in vec3 normal;
in vec2 textCoord;

// ------------------------------------
// ---------- LIGHTING ---------------
// ------------------------------------
subroutine vec3 LightType(vec3 fragKad, vec3 fragKs, vec3 fragNormal, float metallic);
subroutine uniform LightType LightTypeUniform;

uniform vec3 Ia;
uniform vec3 Id;
uniform vec3 Is;

uniform vec3 lightPosition;
uniform vec3 lightDirection;

uniform float cosUmbra;
uniform float cosPenumbra;
uniform float exponentS;

// ----- Lighting attenuation -----
subroutine float AttenuationType(float distance);
subroutine uniform AttenuationType AttenuationTypeUniform;

// Basic model
uniform float c1, c2, c3;

// Ranged distance model
uniform float minDistance, maxDistance;

// Pixar model
uniform float fMax;
uniform float distC, fC;
uniform float exponentSE;
uniform float k0, k1;

// ------------------------------------
// ---------- MATERIALS ---------------
// ------------------------------------
uniform sampler2D texKdSampler;
uniform sampler2D texKsSampler;
uniform float shininess;

layout(location = 0) out vec4 colorFragmento;


// ------------------------------------
// ---------- ATTENUATION -------------
// ------------------------------------

subroutine(AttenuationType)
float basicAttenuation(float distance)
{
	return min(1.0f / (c1 + c2 * distance + c3 * pow(distance, 2)), 1.0f);
}

subroutine(AttenuationType)
float rangedAttenuation(float distance)
{
	return clamp((maxDistance - distance) / (maxDistance - minDistance), 0.0f, 1.0f);
}

subroutine(AttenuationType)
float pixarAttenuation(float distance)
{
	float attenuation = mix(fMax * exp(k0 * pow(distance / distC, -k1)), fC * pow(distC / distance, exponentSE), step(distC, distance));

	return attenuation;
}

// ------------------------------------
// ---------- LIGHTING ---------------
// ------------------------------------

vec3 fresnelSchlick(float cos_theta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cos_theta, 0.0f, 1.0f), 5.0f);
}

float distributionGGX(vec3 n, vec3 h)
{
	float a = shininess * shininess;
	float a2 = a * a;
	float dotNH = max(dot(n, h), 0.0);
	float dotNH2 = dotNH * dotNH;

	float num = a2;
	float denom = (dotNH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return num / denom;
}

float geometrySchlickGGX(float dotNV)
{
	float r = (shininess + 1.0f);
	float k = (r * r) / 8.0f;

	float num = dotNV;
	float denom = dotNV * (1.0f - k) + k;

	return num / denom;
}

float geometrySmith(float dotNV, float dotNL)
{
	float ggx2 = geometrySchlickGGX(dotNV);
	float ggx1 = geometrySchlickGGX(dotNL);

	return ggx1 * ggx2;
}

vec3 getDiffuse(vec3 fragKad, float dotLN)
{
	return Id * fragKad * max(dotLN, 0.0f);
}

vec3 getSpecular(vec3 fragKs, float dotHN)
{
	return Is * fragKs * pow(max(dotHN, 0.0f), shininess);
}

vec3 getDiffuseAndSpecular(vec3 fragKad, vec3 fragKs, vec3 fragNormal, float metallic, vec3 lightDirection)
{
	const vec3 n = normalize(fragNormal);
	const vec3 l = normalize(lightDirection);
	const vec3 v = normalize(-position);
	const vec3 h = normalize(v + l);						// Halfway vector

	const float dotLN = clamp(dot(l, n), -1.0f, 1.0f);      // Prevents Nan values from acos
	const float dotHN = dot(h, n);
	const float dotHV = dot(h, v);
	const float dotNV = dot(n, v);

	vec3 F0 = vec3(.04);
	F0 = mix(F0, fragKad, metallic);

	// BRDF
	float NDF = distributionGGX(n, h);
	float G = geometrySmith(dotNV, dotLN);
	vec3 F = fresnelSchlick(max(dotHV, .0f), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0f) - kS;
	kD *= 1.0f - metallic;

	vec3 numerator = NDF * G * F;
	float denominator = 4.0f * max(dotNV, 0.0f) * max(dotLN, 0.0f) + 0.0001f;
	vec3 specularFactor = clamp(numerator / denominator, .0f, 1.0f);

	const vec3 diffuse = getDiffuse(fragKad, dotLN);
	const vec3 specular = getSpecular(fragKs, dotHN);

	//return diffuse + specular;
	return (Id * (fragKad + Ia) / PI + Is * specularFactor) * max(dotLN, 0.0f);
}

subroutine(LightType)
vec3 ambientLight(vec3 fragKad, vec3 fragKs, vec3 fragNormal, float metallic)
{
	return Ia * fragKad;
}

subroutine(LightType)
vec3 pointLight(vec3 fragKad, vec3 fragKs, vec3 fragNormal, float metallic)
{
	const vec3 diffuseSpecular = getDiffuseAndSpecular(fragKad, fragKs, fragNormal, metallic, lightPosition - position);

	const float distance = distance(lightPosition, position);
	const float attenuation = AttenuationTypeUniform(distance);

	return attenuation * diffuseSpecular;
}

subroutine(LightType)
vec3 directionalLight(vec3 fragKad, vec3 fragKs, vec3 fragNormal, float metallic)
{
	return getDiffuseAndSpecular(fragKad, fragKs, fragNormal, metallic, lightDirection);
}

subroutine(LightType)
vec3 spotLight(vec3 fragKad, vec3 fragKs, vec3 fragNormal, float metallic)
{
	const vec3 diffuseSpecular = getDiffuseAndSpecular(fragKad, fragKs, fragNormal, metallic, lightPosition - position);

	const float distance = distance(lightPosition, position);
	const float distanceAttenuation = AttenuationTypeUniform(distance);

	vec3 l = normalize(lightPosition - position);
	vec3 d = normalize(lightDirection);

	float radialAttenuation = 0.0f, dotLD = dot(-l, d);
	if (dotLD >= cosPenumbra)
	{
		radialAttenuation = 1.0f;
	}
	else if (dotLD > cosUmbra)
	{
		radialAttenuation = pow((dotLD - cosUmbra) / (cosPenumbra - cosUmbra), exponentS);
	}

	return radialAttenuation * distanceAttenuation * diffuseSpecular;
}

subroutine(LightType)
vec3 rimLight(vec3 fragKad, vec3 fragKs, vec3 fragNormal, float metallic)
{
	const vec3 n = normalize(fragNormal);
	const vec3 v = normalize(-position);
	const float vdn = 1.0f - max(dot(v, n), 0.0f);

	return vdn * Ia;
}

// ------------------------------------
// ---------- MATERIALS ---------------
// ------------------------------------

vec4 getKad()
{
	return texture(texKdSampler, textCoord);
}

vec4 getKs()
{
	return texture(texKsSampler, textCoord);
}


void main ()
{
	const vec4 fragKad = getKad();
	const vec4 fragKs = getKs();
	const vec3 fragNormal = normal;
	const float fragMetallic = .5f;

	if (fragKad.w - CUTOFF < .0f) discard;

	const vec3 reflectionColor = LightTypeUniform(fragKad.rgb, fragKs.rgb, fragNormal, fragMetallic);

	colorFragmento = vec4(reflectionColor, fragKad.w);
}