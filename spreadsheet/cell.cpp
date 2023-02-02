#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::Cell(SheetInterface& sheet): impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() {}

void Cell::Set(const std::string& text) {
    std::unique_ptr<Impl> impl;
    
    if (text.empty()) {
        impl = std::make_unique<EmptyImpl>();
    } else if (text.size() > 1 && text.front() == FORMULA_SIGN) {
        try {
            impl = std::make_unique<FormulaImpl>(std::string(text.begin() + 1, text.end()), sheet_); } catch (...) {
            throw FormulaException("Syntax error of formula creation");
        }
    } else {
        impl = std::make_unique<TextImpl>(std::move(text));
    } 
    
    CheckCircularDependency(impl->GetReferencedCells());
    impl_ = std::move(impl);
    UpdateDependencies();
    InvalidateCache(this);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::UpdateDependencies() {
    for (Cell *cell : dependent_cells_) {
        cell->referenced_cells_.erase(this);
    }
    referenced_cells_.clear();

    for (const auto& pos : impl_->GetReferencedCells()) {
        CellInterface *ref_cell = sheet_.GetCell(pos);

        if (!ref_cell) {
            sheet_.SetCell(pos, "");
            ref_cell = sheet_.GetCell(pos);
        }
        referenced_cells_.insert(static_cast<Cell *>(ref_cell));
        static_cast<Cell *>(ref_cell)->dependent_cells_.insert(this);
    }
}

void Cell::CheckCircularDependency(const std::vector<Position>& referenced_cells) {
    std::unordered_set<CellInterface*> visited;
    CheckCircularDependencyHelper(referenced_cells, visited);
}

void Cell::CheckCircularDependencyHelper(const std::vector<Position>& ref_cells, std::unordered_set<CellInterface*>& visited) {
    for (const auto& pos : ref_cells) {
        CellInterface* cell = sheet_.GetCell(pos);
        if (cell == this) {
            throw CircularDependencyException("");
        }
        if (cell && !visited.count(cell)) {
            const auto ref_cells = cell->GetReferencedCells();
            if (!ref_cells.empty())
                CheckCircularDependencyHelper(ref_cells, visited);
            visited.insert(cell);
        }
    }
}

void Cell::InvalidateCache(Cell* cell) {
    for (Cell* dep_cell : cell->dependent_cells_) {
        if (dep_cell->impl_->GetCache()) {
            InvalidateCache(dep_cell);
            dep_cell->impl_->InvalidateCache();
        }
    }
}