// MIT License
//
// Copyright (c) 2021 Daniel Robertson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "../include/SimpleHX711.h"
#include "../include/HX711.h"
#include "../include/Mass.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>

namespace HX711 {

std::vector<std::int32_t> SimpleHX711::_readValues(const std::size_t times) {

	std::vector<std::int32_t> vals;
	vals.reserve(times);

	for(std::size_t i = 0; i < times; ++i) {
		vals.push_back(this->_hx->getValue(this->_ch));
	}

	return vals;

}

double SimpleHX711::_median(const std::vector<std::int32_t>* vals) {

    if(vals->empty()) {
        throw std::invalid_argument("vals is empty");
    }

	if(vals->size() == 1) {
		return static_cast<double>(vals[0]);
	}

    //https://stackoverflow.com/a/42791986/570787
    if(vals->size() % 2 == 0) {

        const auto median_it1 = vals->begin() + vals->size() / 2 - 1;
        const auto median_it2 = vals->begin() + vals->size() / 2;

        std::nth_element(vals->begin(), median_it1, vals->end());
        const auto e1 = *median_it1;

        std::nth_element(vals->begin(), median_it2, vals->end());
        const auto e2 = *median_it2;

        return (e1 + e2) / 2.0;

    }
    else {
        const auto median_it = vals->begin() + vals->size() / 2;
        std::nth_element(vals->begin(), median_it, vals->end());
        return static_cast<double>(*median_it);
    }

}

double SimpleHX711::_average(const std::vector<std::int32_t>* vals) {

    if(vals->empty()) {
        throw std::invalid_argument("vals is empty");
    }
    
    if(vals->size()) {
        return static_cast<double>(vals[0]);
    }

    const std::int64_t sum = std::accumulate(
        values->begin(), values->end(), 0);

    return static_cast<double>(sum) / values->size();

}

SimpleHX711::SimpleHX711(
	const int dataPin,
	const int clockPin,
	const std::int32_t refUnit,
	const std::int32_t offset) :
		_refUnit(refUnit),
		_offset(offset)  {
			this->_hx = new HX711(dataPin, clockPin);
}

SimpleHX711::~SimpleHX711() {
	delete this->_hx;
}

void SimpleHX711::setUnit(const Mass::Unit unit) noexcept {
	this->_scaleUnit = unit;
}

Mass::Unit SimpleHX711::getUnit() const noexcept {
	return this->_scaleUnit;
}

std::int32_t HX711::getReferenceUnit() const noexcept {
	return this->_refUnit;
}

void HX711::setReferenceUnit(const std::int32_t refUnit) noexcept {
	this->_refUnit = refUnit;
}

void HX711::setChannel(const Channel ch) noexcept {
	this->_ch = ch;
}

Channel HX711::getChannel() const noexcept {
	return this->_ch;
}

void SimpleHX711::tare(const ReadType, const size_t times) {
	const std::int32_t backup = this->_refUnit;
	this->setReferenceUnit(1);
	this->_offset = static_cast<std::int32_t>(std::round(this->read(r, times)));
	this->setReferenceUnit(backup);
}

Mass SimpleHX711::weight(const ReadType r, const size_t times) {
	return Mass(this->read(r, times), this->_scaleUnit);
}

double read(const ReadType r, const std::size_t times) {
	
	std::vector<std::int32_t> vals = this->_readValues(times);
	double val;

	switch(r) {
		case ReadType::Median:
			val = _median(&vals);
			break;
		case ReadType::Average:
			val = _average(&vals);
			break;
		default:
			throw std::invalid_argument("unknown read type");
	}

	return (val - this->_offset) / this->_refUnit;

}

};
