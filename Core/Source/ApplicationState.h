#pragma once
#include "Utilities/imgui_color_gradient.h"
#include "Utilities/Typedefs.h"

namespace Nimbus {
	struct ApplicationState {
		enum Rendering { POINT, LINE, TRIANGLE };

		// Application
		bool							_fpsMenuBar;							//!< Show FPS in menu bar
		bool							_closeFlag;								//!< App closing
		bool 							_UIFocus;								//!< The app UI has the mouse focus	
		vec3							_backgroundColor;						//!< Clear color
		vec3							_gridColor;								//!< World grid color
		float							_materialScattering;
		uint16_t						_numFps;
		uvec2							_viewportSize;							//!< Viewport size (!= window)

		// Screenshot
		char							_screenshotFilenameBuffer[32];			//!< Location of screenshot
		float							_screenshotFactor;						//!< Multiplier of current size of GLFW window

		// Rendering type
		Rendering						_visualizationMode;						//!< Only triangle mesh is defined here
		bool							_renderGrid = true;
		bool							_renderMeshletsAABBs = false;
		bool							_renderWithEDL = true;
		float							_edlStrength = 300;
		unsigned						_renderedPoints[3];
		int								_numNeighbors = 1;
		float							_occlusionThreshold = 0.1;
		i32								_pointsBucket = 25000000;

		int								_selectedMeshlet;

		bool							_isPicking = false;

		//Delta time
		float deltaTime = 0.1f;
		float currentFrameTime = 0;
		float lastFrameTime = 0;

		/**
		*	@brief Default constructor.
		*/
		ApplicationState() : _screenshotFilenameBuffer("Screenshot") {
			_backgroundColor = vec3(.2f);
			_gridColor = vec3(0.0f);
			_materialScattering = 1.0f;
			_numFps = 0;
			_viewportSize = vec3(0);

			_screenshotFactor = 1.0f;

			_visualizationMode = TRIANGLE;

			_selectedMeshlet = -1;

		}
	};
}
