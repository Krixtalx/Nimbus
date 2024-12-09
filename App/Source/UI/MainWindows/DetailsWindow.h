#pragma once
#include "UI/I_GUIWindow.h"

namespace Nimbus {
	class Texture;
	class Model3D;
	/**
	 * \brief Inspector selection details window: allow to manipulate scene contents properties
	 */
	class DetailsWindow : public I_GUIWindow {
	protected:
		/**
		 * \brief Shader selection index
		 */
		static int _shaderInspectorShSel;

		/**
		 * \brief Subroutine selection index
		 */
		static int _shaderInspectorSubrutSel;
		
		/**
		 * \brief Uniform selection index
		 */
		static int _shaderInspectorUnifSel;

		/**
		 * \brief Renders inspector details for camera items
		 */
		bool makeCameraItemInspector();

		/**
		 * \brief Renders inspector details for light items
		 */
		bool makeLightItemInspector();

		/**
		 * \brief Renders inspector details for point cloud items
		 */
		bool makePCloudItemInspector() const;

		/**
		 * \brief Renders insepctor details for image texture items
		 */
		bool makeImageItemInspector();

		/**
		 * \briefs Renders inspector details for materials
		 */
		void makeMaterialItemInspector();

		/**
		 * \brief Makes and handles basic enabled and POS/ROT/SCL edit controls
		 */
		bool makeBasicTransformControls(Nimbus::Model3D& model, const std::string& trfIndex = "") const;

		/**
		 * \brief Makes and handles basic texture image view
		 */
		void makeBasicTextureViewer(Texture& selectedTexture);

	public:
		DetailsWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags = 0) :
			I_GUIWindow(id, idx, open, flags) {}
		
		/**
		 * \brief Renders the window and contents
		 */
		void renderWindow() override;

		/**
		 * \brief Resets shader details indices to default
		 */
		static void resetShaderInspector();
	};
}

