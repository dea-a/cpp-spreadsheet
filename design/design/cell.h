#pragma once

#include "common.h"
#include "formula.h"

#include <unordered_set>

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(const std::string& text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    class Impl; //public CellInterface
    class EmptyImpl; //public Impl
    class TextImpl; //public Impl
    struct ValueVisitor {
        CellInterface::Value operator() (double value) {
            return value;
        }

        CellInterface::Value operator() (FormulaError error_code) {
            return error_code;
        }
    };
    class FormulaImpl; //public Impl

    void UpdateDependencies();
    void CheckCircularDependency(const std::vector<Position>& referenced_cells);
    void CheckCircularDependencyHelper(const std::vector<Position>& ref_cells, std::unordered_set<CellInterface*>& visited);
    void InvalidateCache(Cell* cell);

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    std::unordered_set<Cell*> referenced_cells_;
    std::unordered_set<Cell*> dependent_cells_;
};