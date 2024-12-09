#include "UI/I_GUIWindow.h"

namespace Nimbus {

	/**
	 * \brief Application performance windows: shows graphs and values for current and recent performance
	 */
	class StatsWindow : public I_GUIWindow {
	protected:
		/**
		 * \brief Time-based plots values
		 */
		std::vector<double> _timePlot, _fpsPlot, _frametimePlot;
		/**
		 * \brief Plots history times
		 */
		const double _maxTime = 4.0, _halfMaxTime = _maxTime / 2.0;
		/**
		 * \brief FPS plot value range
		 */
		double _minFps, _maxFps;
		/**
		 * \brief Frametime plot value range
		 */
		double _minFramet, _maxFramet;

		void updateStats();

	public:

		StatsWindow(const std::string& id, const LOCALE_STRINGS idx, const bool open, const ImGuiWindowFlags flags = 0) :
			I_GUIWindow(id, idx, open, flags) {
			_minFps = _minFramet = _maxFps = _maxFramet = 0.0;
		}
		
		/**
		 * \brief Renders the window and contents
		 */
		void renderWindow() override;

	};
}
