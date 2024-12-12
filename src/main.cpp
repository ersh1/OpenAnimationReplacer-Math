#include "Conditions.h"

void OnMessage(SKSE::MessagingInterface::Message* a_msg)
{
	if (a_msg->type == SKSE::MessagingInterface::kPostLoad) {
		OAR_API::Conditions::GetAPI(OAR_API::Conditions::InterfaceVersion::V2);
		if (g_oarConditionsInterface) {
			switch (OAR_API::Conditions::AddCustomCondition<Conditions::MathStatementCondition>()) {
				using enum OAR_API::Conditions::APIResult;
			case OK:
				logs::info("Registered {} condition!", Conditions::MathStatementCondition::CONDITION_NAME);
				break;
			case AlreadyRegistered:
				logs::warn("Condition {} is already registered!", Conditions::MathStatementCondition::CONDITION_NAME);
				break;
			case Invalid:
				logs::error("Condition {} is invalid!", Conditions::MathStatementCondition::CONDITION_NAME);
				break;
			case Failed:
				logs::error("Failed to register condition {}!", Conditions::MathStatementCondition::CONDITION_NAME);
				break;
			}
		}
		else {
			logs::error("Failed to request Open Animation Replacer API");
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	if (const auto intfc = SKSE::GetMessagingInterface())
		intfc->RegisterListener(OnMessage);

	return true;
}
