#pragma once
#include<cstring>
#include<string>
#include<iostream>

//class template for char/varchar in SQL
template<int len>
class ConstChar {
public:
	ConstChar(char* data="") {
		strcpy(str_data, data);
	}
	ConstChar(const ConstChar<len> &r) {
		strcpy(str_data, r.str_data);
	}
	ConstChar<len> & operator=(const ConstChar<len> &r) {
		if (this != &r) {
			strcpy(str_data, r.str_data);
		}
		return *this;
	}
	bool operator!=(const ConstChar<len>& r) const {
		return strcmp(str_data, r.str_data);
	}
	bool operator==(const ConstChar<len>& r) const {
		return !(*this != r);
	}
	bool operator<(const ConstChar<len>& r) const {
		return strcmp(str_data, r.str_data) < 0;
	}
	bool operator>(const ConstChar<len>& r) const {
		return (r < *this);
	}
	bool operator<=(const ConstChar<len>& r) const {
		return !(*this > r);
	}
	bool operator>=(const ConstChar<len>& r) const {
		return !(*this < r);
	}
	friend std::istream& operator>>(std::istream& is, ConstChar<len>& obj) {
		is >> obj.str_data;
		return is;
	}
	friend std::ostream& operator<<(std::ostream& os, const ConstChar<len>& obj) {
		os << obj.str_data;
		return os;
	}
private:
	char str_data[len];
};