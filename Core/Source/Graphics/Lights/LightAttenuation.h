#pragma once

#include "LightModel.h"

namespace Nimbus {
	class LightAttenuation {
	public:
		enum AttenuationModel {
			DEFAULT_DISTANCE, CONTROLLED_DISTANCE, PIXAR
		};

	public:
		virtual void applyAttenuation(LightProperties* lightProperties, Nimbus::Material* material) = 0;
	};

	class BasicAttenuation : public LightAttenuation {
	public:
		void applyAttenuation(LightProperties* lightProperties, Nimbus::Material* material) override;
	};

	class DistanceAttenuation : public LightAttenuation {
	public:
		void applyAttenuation(LightProperties* lightProperties, Nimbus::Material* material) override;
	};

	class PixarAttenuation : public LightAttenuation {
	public:
		void applyAttenuation(LightProperties* lightProperties, Nimbus::Material* material) override;
	};
}

