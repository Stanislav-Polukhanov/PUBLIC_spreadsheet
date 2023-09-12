#include "cell.h"

#include <cassert>
#include <string>


Cell::Cell(SheetInterface& sheet)
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>())
{}

Cell::~Cell() = default;

void Cell::Set(std::string text, SheetInterface& sheet, Position pos) {
    std::unique_ptr<Impl> new_cell;
    if (text.empty()){
        new_cell = std::make_unique<EmptyImpl>();
    }
    else if (text.front() == FORMULA_SIGN && text.size() > 1){
        new_cell = std::make_unique<FormulaImpl>(std::move(text), sheet);
        //во всех используемых ячейках делаем пустую, если nullptr - ДЛЯ ТЕСТОВ В MAIN
        std::vector<Position> dependencies = new_cell->GetReferencedCells();
        for (Position cel_pos : dependencies ){
            if (cel_pos.IsValid() && !sheet.GetCell(cel_pos)) {
                sheet.SetCell(cel_pos, "");
            }
        }
        if (CheckCircularDependencies(*new_cell, pos)){
            throw CircularDependencyException("Circular dependency error");
        }
    }
    else {
        new_cell = std::make_unique<TextImpl>(std::move(text));
    }
    impl_.reset();
    impl_ = std::move(new_cell);
    pos_ = pos;
    
    for (CellInterface* dependence : dependencies_){
        dynamic_cast<Cell*>(dependence)->back_dependencies_.erase(this);
    }
    dependencies_.clear();
    for (Position dependence_pos : impl_->GetReferencedCells()){
        if (!sheet.GetCell(dependence_pos)){
            sheet_.SetCell(pos, "");
        }
        dependencies_.insert(sheet.GetCell(dependence_pos));
        dynamic_cast<Cell*>(sheet.GetCell(dependence_pos))->back_dependencies_.insert(this);
    }
    InvalidateCache();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
    //Set("", sheet_, pos_);
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

void Cell::InvalidateCache() {
    impl_->InvalidateCache();
    for (CellInterface* dependence : dependencies_){
        dynamic_cast<Cell*>(dependence)->InvalidateCache();
    }
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !back_dependencies_.empty();
}

bool Cell::CheckCircularDependencies(const Cell::Impl &checked_cell_impl, Position checked_cell_pos) const {
    const std::vector<Position>& referenced_cells = checked_cell_impl.GetReferencedCells();
    std::unordered_set<const CellInterface*> visited_cells;
    for (const Position& pos : referenced_cells){
        if (pos == checked_cell_pos){
            throw CircularDependencyException("Circular dependency error");
        }
        const CellInterface* visited_cell = sheet_.GetCell(pos);
        visited_cells.insert(visited_cell);
        return CheckCircularDependencies(visited_cell, visited_cells, checked_cell_pos);
    }
    return false;
}

bool Cell::CheckCircularDependencies(const CellInterface *checked_cell,
                                     std::unordered_set<const CellInterface *> &visited_cells,
                                     Position checked_cell_pos) const {
    for (Position pos : checked_cell->GetReferencedCells()){
        if (pos == checked_cell_pos){
            throw CircularDependencyException("Circular dependency error");
        }
        const CellInterface* visited_cell = sheet_.GetCell(pos);
        if (!visited_cells.count(visited_cell)){
            visited_cells.insert(visited_cell);
            return CheckCircularDependencies(visited_cell, visited_cells, checked_cell_pos);
        }
    }
    return false;
}

[[nodiscard]] std::string Cell::EmptyImpl::GetText() const {
    return {};
}

[[nodiscard]] Cell::Value Cell::EmptyImpl::GetValue() const {
    return GetText();
}

Cell::TextImpl::TextImpl(std::string&& string_input)
    : text_(std::move(string_input))
{}

[[nodiscard]] std::string Cell::TextImpl::GetText() const {
    return text_;
}

[[nodiscard]] Cell::Value Cell::TextImpl::GetValue() const {
    assert(!text_.empty());
    if (text_.front() == ESCAPE_SIGN){
        return text_.substr(1);
    }
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string&& string_input, SheetInterface& sheet)
    : formula_(ParseFormula(string_input.substr(1)))
    , sheet_(sheet)
{}

[[nodiscard]] std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}

[[nodiscard]] Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_){
        cache_ = formula_->Evaluate(sheet_);
    }
    if (std::holds_alternative<double>(cache_.value())) {
        return std::get<double>(cache_.value());
    }
    else {
        return std::get<FormulaError>(cache_.value());
    }
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

//bool Cell::FormulaImpl::HasCache() {
//    return cache_.has_value();
//}

void Cell::FormulaImpl::InvalidateCache() {
    cache_.reset();
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

bool Cell::Impl::HasCache() {
    return false;
}
