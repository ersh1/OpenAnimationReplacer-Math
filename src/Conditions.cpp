#include "Conditions.h"

#include "API/OpenAnimationReplacerAPI-UI.h"

namespace Conditions
{
	MathConditionComponent::MathVariable::MathVariable(const ICondition* a_parentCondition, const char* a_name)
	{
		const auto numericConditionComponentFactory = g_oarConditionsInterface->GetConditionComponentFactory(
			ConditionComponentType::kNumeric);
		component = std::unique_ptr<INumericConditionComponent>(
			static_cast<INumericConditionComponent*>(numericConditionComponentFactory(a_parentCondition,
				a_name, "A variable from the math statement.")));
	}

	void MathConditionComponent::InitializeComponent(void* a_value)
	{
		auto& val = *static_cast<rapidjson::Value*>(a_value);

		auto object = val.GetObj();

		auto mathIt = object.FindMember(rapidjson::StringRef(_name.data(), _name.length()));
		if (mathIt != object.MemberEnd() && mathIt->value.IsObject())
		{
			auto mathObject = mathIt->value.GetObj();
			auto expressionIt = mathObject.FindMember("expression");
			if (expressionIt != mathObject.MemberEnd() && expressionIt->value.IsString())
			{
				_expressionString = expressionIt->value.GetString();
			}

			// set the expression, creating the necessary subcomponents
			SetExpression(_expressionString.data());

			// initialize all subcomponents that were just created
			auto variablesIt = mathObject.FindMember("variables");
			if (variablesIt != mathObject.MemberEnd() && variablesIt->value.IsObject())
			{
				for (auto& variable : _variables)
				{
					variable->component->InitializeComponent(&variablesIt->value);
				}
			}
		}
	}

	void MathConditionComponent::SerializeComponent(void* a_value, void* a_allocator)
	{
		auto& val = *static_cast<rapidjson::Value*>(a_value);
		auto& allocator = *static_cast<rapidjson::Document::AllocatorType*>(a_allocator);

		rapidjson::Value mathValue(rapidjson::kObjectType);
		mathValue.AddMember("expression", rapidjson::StringRef(_expressionString.data(), _expressionString.length()),
		                    allocator);

		rapidjson::Value variablesValue(rapidjson::kObjectType);
		for (auto& variable : _variables)
		{
			variable->component->SerializeComponent(&variablesValue, a_allocator);
		}
		mathValue.AddMember("variables", variablesValue, allocator);

		val.AddMember(rapidjson::StringRef(_name.data(), _name.length()), mathValue, allocator);
	}

	bool MathConditionComponent::DisplayInUI(bool a_bEditable, float a_firstColumnWidthPercent)
	{
		if (!OAR_API::UI::GetAPI())
		{
			return false;
		}

		if (!OAR_API::UI::IsImGuiContextInitialized())
		{
			if (!OAR_API::UI::InitializeImGuiContext())
			{
				return false;
			}
		}

		bool bEdited = false;

		if (a_bEditable)
		{
			ImGui::PushID(&_expressionString);
			ImGui::SetNextItemWidth(g_oarUIInterface->GetFirstColumnWidth(a_firstColumnWidthPercent));
			if (ImGui::InputTextWithHint("##Math", "Math expression...", &_expressionString,
			                             ImGuiInputTextFlags_EnterReturnsTrue))
			{
				SetExpression(_expressionString.data());
				bEdited = true;
			}
			ImGui::PopID();
		}
		else
		{
			ImGui::TextUnformatted(_expressionString.data());
		}

		// display subcomponents
		std::string subComponentTableId = std::format("{}subComponentTable", (uintptr_t)&_variables);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2, 2));
		if (ImGui::BeginTable(subComponentTableId.data(), 1, ImGuiTableFlags_Borders))
		{
			for (auto& variable : _variables)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				const auto componentName = variable->component->GetName();
				auto savedCursorPos = ImGui::GetCursorPos();
				ImGui::SetCursorPosX(
					ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(componentName.data()).x - ImGui::GetStyle().
					FramePadding.x);
				ImGui::TextDisabled(componentName.data());
				// show component description on mouseover
				const auto componentDescription = variable->component->GetDescription();
				if (!componentDescription.empty())
				{
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
					{
						ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
						ImGui::TextUnformatted(componentDescription.data());
						ImGui::PopTextWrapPos();
						ImGui::EndTooltip();
						ImGui::PopStyleVar();
					}
				}

				ImGui::SetCursorPos(savedCursorPos);

				if (variable->component->DisplayInUI(a_bEditable, a_firstColumnWidthPercent))
				{
					bEdited = true;
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		return bEdited;
	}

	float MathConditionComponent::GetExpressionResult(RE::TESObjectREFR* a_refr) const
	{
		for (const auto& variable : _variables)
		{
			variable->SetValue(a_refr);
		}

		return _expression.value();
	}

	void MathConditionComponent::SetExpression(const char* a_expression)
	{
		_expression = exprtk::expression<float>();
		_symbolTable = exprtk::symbol_table<float>();

		exprtk::parser<float> parser;
		std::vector<std::string> variableNames;

		if (exprtk::collect_variables(a_expression, variableNames))
		{
			// remove no longer used variables
			for (auto it = _variables.begin(); it != _variables.end();)
			{
				auto search = std::ranges::find_if(variableNames, [&](const auto& a_variableName)
				{
					return a_variableName == std::string_view(
						(*it)->component->GetName());
				});
				if (search == variableNames.end())
				{
					it = _variables.erase(it);
				}
				else
				{
					++it;
				}
			}

			// add new variables
			for (auto& varName : variableNames)
			{
				auto search = std::ranges::find_if(_variables, [&](const auto& a_variable)
				{
					return varName == std::string_view(
						a_variable->component->GetName());
				});
				if (search != _variables.end())
				{
					_symbolTable.add_variable(varName, (*search)->value);
				}
				else
				{
					auto& newVar = _variables.emplace_back(std::make_unique<MathVariable>(GetParentCondition(), varName.data()));
					_symbolTable.add_variable(varName, newVar->value);
				}
			}

			// sort variables
			std::ranges::sort(_variables, [&](const auto& a_lhs, const auto& a_rhs)
			{
				auto it_lhs = std::ranges::find(variableNames, std::string_view(a_lhs->component->GetName()));
				auto it_rhs = std::ranges::find(variableNames, std::string_view(a_rhs->component->GetName()));
				return it_lhs < it_rhs;
			});
		}

		_expression.register_symbol_table(_symbolTable);
		parser.compile(a_expression, _expression);
	}

	IConditionComponent* MathConditionComponentFactory(const ICondition* a_parentCondition, const char* a_name, const char* a_description)
	{
		return new MathConditionComponent(a_parentCondition, a_name, a_description);
	}

	MathStatementCondition::MathStatementCondition()
	{
		mathComponent = static_cast<MathConditionComponent*>(_wrappedCondition->AddComponent(
			MathConditionComponentFactory, "Math Statement"));
	}

	bool MathStatementCondition::EvaluateImpl(RE::TESObjectREFR* a_refr, RE::hkbClipGenerator*, void*) const
	{
		return mathComponent->GetExpressionResult(a_refr) != 0.f;
	}
}
