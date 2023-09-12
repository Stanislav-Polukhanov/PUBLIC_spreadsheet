#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <functional>
#include <iostream>

using namespace std::literals;


void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid cell coordinates");
    }
    if (table_.size() <= uint(pos.row)){
        table_.resize(pos.row + 1);
        table_[pos.row].resize(pos.col + 1);
    }
    if (table_[pos.row].size() <= uint(pos.col)){
        table_[pos.row].resize(pos.col + 1);
    }
    if (!table_[pos.row][pos.col]){
        table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }
    table_[pos.row][pos.col]->Set(text, *this, pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid cell coordinates");
    }
    if (pos.row < int(std::size(table_)) && pos.col < int(std::size(table_[pos.row]))) {
        return table_[pos.row][pos.col].get();
    }
    return nullptr;
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid cell coordinates");
    }
    if (pos.row < int(std::size(table_)) && pos.col < int(std::size(table_[pos.row]))) {
        if (!table_[pos.row][pos.col].get() || table_[pos.row][pos.col].get()->GetText().empty()) {
            return nullptr;
        }
        else {
            return table_[pos.row][pos.col].get();
        }
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid cell coordinates");
    }
    
    if (uint(pos.row) >= table_.size() || uint(pos.col) >= table_.at(pos.row).size()){
        return;
    }
    if (table_.size() > uint(pos.row)){
        if (table_.at(pos.row).size() > uint(pos.col)){
            if (table_[pos.row][pos.col]) {
                table_[pos.row][pos.col]->InvalidateCache();
                table_[pos.row][pos.col].reset();
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size output;
    for (int row = 0; uint(row) < table_.size(); ++row) {
        for (int col = int(table_.at(row).size()) - 1; col >= 0; --col) {
            if (table_[row][col]){
                if (table_[row][col]->GetText().empty()) {
                    continue;
                }
                else {
                    output.rows = std::max(output.rows, row + 1);
                    output.cols = std::max(output.cols, col + 1);
                    break;
                }
            }
        }
    }
    return output;
}

// Выводит всю таблицу в переданный поток. Столбцы разделяются знаком
// табуляции. После каждой строки выводится символ перевода строки. Для
// преобразования ячеек в строку используются методы GetValue() или GetText()
// соответственно. Пустая ячейка представляется пустой строкой в любом случае.
void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {
            if (col != 0) {
                output << '\t';
            }
            if (uint(col) < table_[row].size()){
                if (table_[row][col]) {
                    auto cell_value = table_[row][col]->GetValue();
                    if (std::holds_alternative<std::string>(cell_value)) {
                        output << std::get<std::string>(cell_value);
                    }
                    else if (std::holds_alternative<double>(cell_value)){
                        output << std::get<double>(cell_value);
                    }
                    else {
                        output << std::get<FormulaError>(cell_value);
                    }
                }
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {
            if (col != 0) {
                output << '\t';
            }
            if (uint(col) < table_[row].size()){
                if (table_[row][col]) {
                    output << table_[row][col]->GetText();
                }
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}