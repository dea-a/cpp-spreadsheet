#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
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
    class Impl : public CellInterface {
    public:
        std::vector<Position> GetReferencedCells() const {
            return {};
        }
        
        virtual std::optional<FormulaInterface::Value> GetCache() const {
            return std::nullopt;
        }
        
        virtual void InvalidateCache(){}
    };
    
    class EmptyImpl : public Impl {
    public:    
        Value GetValue() const  override {
            return Value{};
        }

        std::string GetText() const override {
            return std::string();
        }
    };
    
    class TextImpl : public Impl {
    public:
        TextImpl(std::string text)
            : text_(std::move(text)) {
        }

        Value GetValue() const  override {
            if (!text_.empty() && text_.front() == ESCAPE_SIGN) {
                return text_.substr(1);
            }
            return text_;
        }

        std::string GetText() const override {
            return text_;
        }
    private:
        std::string text_;  
    };
    
    struct ValueVisitor {
        CellInterface::Value operator() (double value) {
            return value;
        }

        CellInterface::Value operator() (FormulaError error_code) {
            return error_code;
        }
    };
    
    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string text, const SheetInterface &sheet)
            : sheet_(sheet) 
            , formula_(ParseFormula(text)) {
        }

        Value GetValue() const  override {
            if (!cache_) {
                cache_ = formula_->Evaluate(sheet_);
            }
            return std::visit(ValueVisitor{}, *cache_);
        }

        std::string GetText() const override {
            return FORMULA_SIGN + formula_->GetExpression();
        }
        
        std::vector<Position> GetReferencedCells() const {
            return formula_->GetReferencedCells();
        }
        
        void InvalidateCache() override {
            cache_.reset();
        }
        
        std::optional<FormulaInterface::Value> GetCache() const override {
            return cache_;
        }
    private:
        const SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };
    
    void UpdateDependencies();
    void CheckCircularDependency(const std::vector<Position>& referenced_cells);
    void CheckCircularDependencyHelper(const std::vector<Position>& ref_cells, std::unordered_set<CellInterface*>& visited);
    void InvalidateCache(Cell* cell);

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    std::unordered_set<Cell*> referenced_cells_;
    std::unordered_set<Cell*> dependent_cells_;
};