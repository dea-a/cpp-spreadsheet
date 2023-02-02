#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit([&output](auto& val) {
        output << val;
    }, value);
    return output;
}

Sheet::~Sheet() {}

void Sheet::TestValidPosition(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
}

int Sheet::GetTableSize() const {
    return static_cast<int>(table_.size());
}

int Sheet::GetTableColSize(int row) const {
    return static_cast<int>(table_[row].size());
}

void Sheet::SetCell(Position pos, const std::string& text) {
    TestValidPosition(pos);
    SetPrintableSize(pos);
    
    auto &cell = table_[pos.row][pos.col];
    if (!cell) {
        cell = std::make_unique<Cell>(*this);
    }
    cell->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    TestValidPosition(pos);

    if (pos.row >= GetTableSize() ||
        pos.col >= GetTableColSize(pos.row)) {
        return nullptr;
    }

    return table_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    TestValidPosition(pos);
    
    if (pos.row < GetTableSize() &&
        pos.col < GetTableColSize(pos.row)) {
        if (auto &cell = table_[pos.row][pos.col]) {
            cell->Clear();
            cell.reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size size{0, 0};
    for (int row = 0; row < GetTableSize(); ++row) {
        for (int col = GetTableColSize(row) - 1; col >= 0; --col) {
            if (const auto &cell = table_[row][col]) {
                if (!cell->GetText().empty()) {
                    size.rows = std::max(size.rows, row + 1);
                    size.cols = std::max(size.cols, col + 1);
                    break;
                }
            }
        }
    }
    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    auto size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        int last_cell = GetTableColSize(row);

        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }

            if (col < last_cell) {
                if (const auto &cell = table_[row][col]) {
                    output << cell->GetValue();
                }
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    auto size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        int last_cell = GetTableColSize(row);

        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }

            if (col < last_cell) {
                if (const auto &cell = table_[row][col]) {
                    output << cell->GetText();
                }
            }
        }
        output << '\n';
    }
}

void Sheet::SetPrintableSize(Position pos) {
    table_.resize(std::max(pos.row + 1, GetTableSize()));
    table_[pos.row].resize(std::max(pos.col + 1, GetTableColSize(pos.row)));
}


std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}