#include "common.h"

#include <cctype>
#include <cmath>
#include <stdexcept>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
    return this->row == rhs.row && this->col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    if (col >= Position::MAX_COLS || row >= Position::MAX_ROWS
    || col < 0 || row < 0){
        return false;
    }
    return true;
}

[[nodiscard]] std::string Position::ToString() const {
    if (!this->IsValid()){
        return {};
    }
    std::string output;

    int columns_temp = col;
    while (columns_temp >= 0) {
        output.insert(output.begin(), 'A' + columns_temp % LETTERS);
        columns_temp = columns_temp / LETTERS - 1;
    }

    output += std::to_string(row + 1);
    return output;
}

[[nodiscard]] Position Position::FromString(std::string_view str) {
    if (std::isdigit(str.front()) || !std::isdigit(str.back())){ //первый символ - буква, последний - цифра
        return Position::NONE;
    }
    Position output;
    int first_digit_pos = 0;
    bool last_symbol_is_char = true;
    for (size_t i = 0; i < str.size(); ++i){
        if (!std::isdigit(str[i])){
            if (!last_symbol_is_char){
                return Position::NONE;
            }
            last_symbol_is_char = true;
        }
        if (std::isdigit(str[i])){
            if (!first_digit_pos) {
                first_digit_pos = i;
            }
            last_symbol_is_char = false;
        }
    }
    std::string_view letters = str.substr(0, first_digit_pos);
    std::string_view digits = str.substr(first_digit_pos, str.size() - first_digit_pos);
    for (size_t i = 0; i < letters.size(); ++i){
        if (letters[i] < 'A' || letters[i] > 'Z'){
            return Position::NONE;
        }
        output.col += (letters[i] - 'A' + 1) * std::pow(LETTERS, letters.size() - i - 1);
    }
    try {
        output.row = std::stoi(std::string(digits));
    } catch (std::out_of_range&) {
        return Position::NONE;
    }
    --output.col;
    --output.row;
    if (output.IsValid()){
        return output;
    }
    else {
        return Position::NONE;
    }
}