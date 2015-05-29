
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_MEASRANGE_H
#define POLYGRAPH__RUNTIME_MEASRANGE_H

// a [min, max] range of measurements or desired properties
template <class Base>
class MeasRange {
	public:
		inline MeasRange();

		inline bool known() const;
		inline bool contains(const Base &meas) const;

		inline void min(const Base &meas);
		inline void max(const Base &meas);

		inline ostream &print(ostream &os) const;

	protected:
		Base theMin;
		Base theMax;
		bool isMinSet;
		bool isMaxSet;
};


template <class Base>
inline
ostream &operator <<(ostream &os, const MeasRange<Base> &measRange) {
	return measRange.print(os);
}


template <class Base>
inline
MeasRange<Base>::MeasRange(): isMinSet(false), isMaxSet(false) {
}

template <class Base>
inline
bool MeasRange<Base>::known() const {
	return isMinSet || isMaxSet;
}

template <class Base>
inline
bool MeasRange<Base>::contains(const Base &meas) const {
	return 
		(!isMinSet || theMin <= meas) && 
		(!isMaxSet || meas <= theMax);
}

template <class Base>
inline
void MeasRange<Base>::min(const Base &meas) {
	theMin = meas;
	isMinSet = true;
}

template <class Base>
inline
void MeasRange<Base>::max(const Base &meas) {
	theMax = meas;
	isMaxSet = true;
}

template <class Base>
inline 
ostream &MeasRange<Base>::print(ostream &os) const {
	if (isMinSet && isMaxSet)
		os << "from " << theMin << " to " << theMax;
	else
	if (isMinSet)
		os << "at least " << theMin;
	else
	if (isMaxSet)
		os << "at most " << theMax;
	else
		os << "any";
	return os;
}

#endif
