#include "CorePch.h"
#include "MaterialList.h"

Nimbus::MaterialList::~MaterialList() {
	for (const auto& pair : _materials) {
		delete pair.second;
	}
}

Nimbus::Material* Nimbus::MaterialList::isMaterialAvailable(const std::string& name) {
	const auto it = _materials.find(name);

	return it != _materials.end() ? it->second : nullptr;
}

void Nimbus::MaterialList::saveMaterial(const std::string& name, Nimbus::Material* material) {
	if (material->_name != name) {
		material->_name = name;
	}
	if (_materials.contains(name)) {
		_materials[name]->_associatedShader = material->_associatedShader;
	} else {
		_materials[name] = material;
	}
}

void Nimbus::MaterialList::clear() {
	_materials.clear();
}
