#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include "AdditionalFunctions.h"

template<class _Ty, class = std::enable_if<is_character_type<_Ty>::value>::type> class StringBuilder {
private:
	std::basic_string<_Ty> result;

public:
	StringBuilder() {
		result.clear();
	}
	~StringBuilder() {
		result.clear();
	}

	void append(const bool value) {
		_Ty array[5] = { 0x00 };
		std::string boolValue = value == true ? "true" : "false";

		this->result += std::basic_string<_Ty>(boolValue.begin(), boolValue.end());
	}

	void append(const _Ty c) {
		this->result += c;
	}

	void append(const int value) {
		char array[32] = { 0x00 };
		sprintf(array, "%i", value);

		std::string resultValue = std::string(array);

		this->result += std::basic_string<_Ty>(resultValue.begin(), resultValue.end());
	}

	void append(const dword_t value) {
		char array[32] = { 0x00 };
		sprintf(array, "%u", value);

		std::string resultValue = std::string(array);

		this->result += std::basic_string<_Ty>(resultValue.begin(), resultValue.end());
	}

	void append(const double d) {
		char array[128] = { 0x00 };
		sprintf(array, "%f", d);

		std::string resultValue = std::string(array);

		this->result += std::basic_string<_Ty>(resultValue.begin(), resultValue.end());
	}

	void append(const _Ty *str) {
		std::basic_string<_Ty> resultValue = "";
		if (str == nullptr) {
			std::string nullstr = "null";
			resultValue = std::basic_string<_Ty>(nullstr.begin(), nullstr.end());
		}
		else {
			resultValue = str;
		}
		this->result += std::basic_string<_Ty>(resultValue.begin(), resultValue.end());
	}

	void insert(const dword_t idx, const bool value) {
		_Ty array[5] = { 0x00 };
		std::string boolValue = value == true ? "true" : "false";

		this->result = this->result.substr(0, idx) + std::basic_string<_Ty>(boolValue.begin(), boolValue.end()) + this->result.substr(idx + 1);
	}

	void insert(const dword_t idx, const dword_t value) {
		char array[32] = { 0x00 };
		sprintf(array, "%i", value);

		std::string resultValue = std::string(array);
		this->result = this->result.substr(0, idx) + std::basic_string<_Ty>(resultValue.begin(), resultValue.end()) + this->result.substr(idx + 1);
	}

	void insert(const dword_t idx, const double d) {
		char array[64] = { 0x00 };
		sprintf(array, "%f", d);

		std::string resultValue = std::string(array);
		this->result = this->result.substr(0, idx) + std::basic_string<_Ty>(resultValue.begin(), resultValue.end()) + this->result.substr(idx + 1);
	}

	void insert(const dword_t idx, const _Ty *str) {
		std::basic_string<_Ty> resultValue = "";
		if (str == nullptr) {
			std::string nullstr = "null";
			resultValue = std::basic_string<_Ty>(nullstr.begin(), nullstr.end());
		}
		else {
			resultValue = str;
		}
		this->result = this->result.substr(0, idx) + std::basic_string<_Ty>(resultValue.begin(), resultValue.end()) + this->result.substr(idx + 1);
	}

	void remove(const dword_t idx, const dword_t len) {
		if (idx + len >= this->result.length()) {
			return;
		}
		this->result = this->result.substr(0, idx) + this->result.substr(idx + len);
	}

	const _Ty getCharAt(const dword_t idx) const {
		if (this->result.length() < idx) {
			return 0x00;
		}
		return this->result.at(idx);
	}

	const dword_t getCapacity() const {
		return this->result.capacity();
	}

	const dword_t getLength() const {
		return this->result.length();
	}

	std::basic_string<_Ty> substr(const dword_t idx) const {
		return this->result.substr(idx);
	}

	std::basic_string<_Ty> substr(const dword_t idx, const dword_t len) const {
		return this->result.substr(idx, len);
	}

	const _Ty* toStringChar() const {
		return this->result.c_str();
	}

	const std::basic_string<_Ty>& toString() const {
		return this->result;
	}
};