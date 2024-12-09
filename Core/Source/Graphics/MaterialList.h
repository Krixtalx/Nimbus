#pragma once

#include "Material.h"
#include "Utilities/Singleton.h"

namespace Nimbus {
	class MaterialList : public Singleton<MaterialList> {
		friend Singleton;

		std::unordered_map<std::string, Nimbus::Material*> _materials;

		MaterialList() = default;
	public:
		virtual ~MaterialList();

		Nimbus::Material* isMaterialAvailable(const std::string& name);
		void saveMaterial(const std::string& name, Nimbus::Material* material);

		void clear();

		std::unordered_map<std::string, Nimbus::Material*>::const_iterator getBegin() const { return _materials.cbegin(); }
		std::unordered_map<std::string, Nimbus::Material*>::const_iterator getEnd() const { return _materials.cend(); }
	};
}

