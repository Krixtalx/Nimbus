#pragma once
#include "Geometry/AABB.h"
#include <string>

namespace Nimbus {
	class Material;

	class Model3D {
		friend class DetailsWindow;
		friend class Renderer;

	protected:
		std::string		_name;
		dvec3			_pos;
		dvec3			_rot;
		dvec3			_scale;
		dvec3			_offset;
		bool			_enabled;
		AABB			_aabb;
		Material*		_material;
	public:
		explicit Model3D(const std::string& name = "Model3D", const vec3& pos = { 0,0,0 }, const vec3& rot = { 0,0,0 }, const vec3& scale = { 1,1,1 }, const vec3& offset = {0, 0, 0});
		Model3D(const std::string& name, const AABB& aabb, const vec3& pos = { 0,0,0 }, const vec3& rot = { 0,0,0 }, const vec3& scale = { 1,1,1 }, const vec3& offset = { 0, 0, 0 });

		virtual void drawModel(dmat4 viewProjMatrix) = 0;
		virtual void saveBinaryFile(const std::string& path) = 0;
		virtual void loadBinaryFile(const std::string& path) = 0;

		void move(const vec3& offset);
		void rotate(const vec3& offset);
		void scale(const vec3& offset);
		void setPosition(const vec3& newPosition);
		void setRotation(const vec3& newRotation);
		void setScale(const vec3& newScale);
		void setOffset(const vec3& newOffset);
		void setEnable(bool enable);
		void setAABB(const AABB& newAABB);
		virtual void moveGeometryToOrigin();
		dmat4 getModelMatrix() const;
		dmat4 getMVPMatrix(const dmat4& vpMatrix) const;
		virtual AABB getAABB();
		std::string getName() const;
		Material* getMaterial() const;
		dvec3 getPosition() const;
		dvec3 getOffset() const;
	};
}
