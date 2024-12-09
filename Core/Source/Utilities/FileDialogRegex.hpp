#pragma once

#include <map>

namespace FileDialogRegex {
	enum class FileDialogType {
		POINT_CLOUDS,
		IMAGES,
		IMAGES_PNG,
		IMAGES_TIF,
		TEXT,
		XYZ,
		SCENE,
		DIRECTORY,
		ANY
	};
	static inline std::map<FileDialogType, const char*> FILEDIALOG_REGEX = {
			{FileDialogType::POINT_CLOUDS, "LAS/LAZ (*.las *.laz){.((las|LAS)),.((laz|LAZ))},PLY (*.ply){.((ply|PLY))}"},
			{FileDialogType::IMAGES, "PNG (*.png){.((png|PNG))},JPG/JPEG (*.jpg, *.jpeg){.((jpg|JPG)),.((jpeg|JPEG))},TIF/TIFF (*.tif,*.tiff){.((tiff|TIFF)),.((tif|TIF))}"},
			{FileDialogType::IMAGES_PNG, "PNG (*.png){.((png|PNG))}"},
			{FileDialogType::IMAGES_TIF, "TIF (*.tif *.tiff){.((tiff|TIFF)),.((tif|TIF))}"},
			{FileDialogType::TEXT, "TXT (*.txt){.((txt|TXT))}"},
			{FileDialogType::XYZ, "XYZ (*.xyz){.((xyz|XYZ))}"},
			{FileDialogType::SCENE, "Nimbus Scene (*.Nimbus){.((nimbus|NIMBUS))}"},
			{FileDialogType::DIRECTORY, ""},
			{FileDialogType::ANY, "* {.*}"}
	};

	inline const char* getRegex(const FileDialogType type) {
		return FILEDIALOG_REGEX[type];
	}

}
