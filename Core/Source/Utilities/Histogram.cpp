#include "CorePch.h"
#include "Histogram.h"


// [Public methods]

Histogram::Histogram(std::vector<float>* data) : _data(data) {
}

void Histogram::buildHistogram(const unsigned numBins, const float minValue, const float maxValue, const bool density) {
	_histogram.resize(numBins);
	_boundary = vec2(minValue, maxValue);
	std::fill(_histogram.begin(), _histogram.end(), 0);

	for (float value : *_data) {
		float scaledValue = (value - minValue) / (maxValue - minValue) * (numBins - 1.0f);
		++_histogram[std::round(scaledValue)];
	}

	if (density) {
		for (int histIndex = 0; histIndex < numBins; ++histIndex) {
			_histogram[histIndex] /= _data->size();
		}
	}
}

void Histogram::exportLatex(const std::string& filename) const {
	std::string latexLine = "{ ";

	for (int histIndex = 0; histIndex < _histogram.size(); ++histIndex) {
		latexLine += "(" + std::to_string((histIndex * (_boundary.y - _boundary.x) / _histogram.size()) + _boundary.x) + ", " + std::to_string(_histogram[histIndex]) + ") ";
	}

	latexLine += "}";

	{
		std::ofstream out(filename);
		out << latexLine;
		out.close();
	}
}

void Histogram::measureDistance(Histogram* compHistogram) {
	if (compHistogram->_histogram.size() != this->_histogram.size()) return;

	std::cout << "Bhattacharyya distance: " << Histogram::bhattacharyyaDistance(this, compHistogram) << std::endl;
	std::cout << "Chi-Square distance: " << Histogram::chiSquareDistance(this, compHistogram) << std::endl;
	std::cout << "Intersection distance: " << Histogram::intersectionDistance(this, compHistogram) << std::endl;
}

// [Static methods]

float Histogram::bhattacharyyaDistance(Histogram* histogram1, Histogram* histogram2) {
	float distance = .0f, sum1 = .0f, sum2 = .0f;

	for (int binIdx = 0; binIdx < histogram1->_histogram.size(); ++binIdx) {
		distance += sqrt(histogram1->_histogram[binIdx] * histogram2->_histogram[binIdx]);
		sum1 += histogram1->_histogram[binIdx];
		sum2 += histogram2->_histogram[binIdx];
	}

	return sqrt(1.0f - (1.0f / sqrt(sum1 * sum2 * histogram1->_histogram.size() * histogram2->_histogram.size())) * distance);
}

float Histogram::chiSquareDistance(Histogram* histogram1, Histogram* histogram2) {
	float distance = .0f;

	for (int binIdx = 0; binIdx < histogram1->_histogram.size(); ++binIdx) {
		if (histogram1->_histogram[binIdx] > glm::epsilon<float>())
			distance += pow(histogram1->_histogram[binIdx] - histogram2->_histogram[binIdx], 2.0) / histogram1->_histogram[binIdx];
	}

	return distance;
}

float Histogram::intersectionDistance(Histogram* histogram1, Histogram* histogram2) {
	float distance = .0f;

	for (int binIdx = 0; binIdx < histogram1->_histogram.size(); ++binIdx) {
		distance += std::min(histogram1->_histogram[binIdx], histogram2->_histogram[binIdx]);
	}

	return distance;
}
