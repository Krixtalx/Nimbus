#include "CorePch.h"
#include "InterfaceAdapter.h"

Nimbus::InterfaceAdapter::InterfaceAdapter(const std::string& id, const std::string& icon, LOCALE_STRINGS idx, const vec2& sizeConstaintsMin, const vec2& sizeConstaintsMax) :
	_methodId("###" + id), _icon(icon), _locNameIdx(idx), _windowOpen(true), _sizeConstraintsMin(sizeConstaintsMin), _sizeConstraintsMax(sizeConstaintsMax) {}
