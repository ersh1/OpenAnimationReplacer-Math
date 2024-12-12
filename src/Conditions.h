#pragma once

#include "API/OpenAnimationReplacerAPI-Conditions.h"

namespace Conditions
{
	class MathConditionComponent : public ICustomConditionComponent
	{
	public:
		inline static constexpr auto DEFAULT_DESCRIPTION = "A math expression (e.g. x + y > 2)."sv;

		struct MathVariable
		{
			MathVariable(const ICondition* a_parentCondition, const char* a_name);

			void SetValue(RE::TESObjectREFR* a_refr) const
			{
				value = component->GetNumericValue(a_refr);
			}

			std::unique_ptr<INumericConditionComponent> component;
			mutable float value = 0.f;
		};

		MathConditionComponent(const ICondition* a_parentCondition, const char* a_name, const char* a_description = "") :
			ICustomConditionComponent(a_parentCondition, a_name, a_description)
		{
			_expression.register_symbol_table(_symbolTable);
		}

		void InitializeComponent(void* a_value) override;
		void SerializeComponent(void* a_value, void* a_allocator) override;

		bool DisplayInUI(bool a_bEditable, float a_firstColumnWidthPercent) override;

		RE::BSString GetArgument() const override { return _expressionString.data(); }
		RE::BSString GetDefaultDescription() const override { return DEFAULT_DESCRIPTION; }

		bool IsValid() const override { return true; }

		float GetExpressionResult(RE::TESObjectREFR* a_refr) const;
		void SetExpression(const char* a_expression);

		[[nodiscard]] std::string_view GetExpression() const { return _expressionString; }

	private:
		std::string _expressionString{};
		std::vector<std::unique_ptr<MathVariable>> _variables;
		exprtk::expression<float> _expression;
		exprtk::symbol_table<float> _symbolTable;
	};

	IConditionComponent* MathConditionComponentFactory(const ICondition* a_parentCondition, const char* a_name, const char* a_description);

	class MathStatementCondition : public CustomCondition
	{
	public:
		constexpr static inline std::string_view CONDITION_NAME = "MathStatement"sv;

		MathStatementCondition();

		RE::BSString GetArgument() const override { return mathComponent->GetArgument(); }

		RE::BSString GetName() const override { return CONDITION_NAME.data(); }

		RE::BSString GetDescription() const override
		{
			return "Checks if a math statement is true (results in something other than 0)."sv.data();
		}

		constexpr REL::Version GetRequiredVersion() const override { return {1, 0, 0}; }

		MathConditionComponent* mathComponent;

	protected:
		bool EvaluateImpl(RE::TESObjectREFR* a_refr, RE::hkbClipGenerator* a_clipGenerator, void* a_subMod) const override;
	};
}
