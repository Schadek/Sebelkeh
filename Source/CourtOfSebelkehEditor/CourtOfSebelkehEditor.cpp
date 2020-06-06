#include "CourtOfSebelkehEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

#include "CourtOfSebelkeh/Skills/SkillBase.h"
#include "CourtOfSebelkeh/Buffs/BuffSystemDefinitions.h"
#include "CourtOfSebelkehEditor/ThumbnailRenderer/SkillThumbnailRenderer.h"
#include "ThumbnailRendering/ThumbnailManager.h"

IMPLEMENT_GAME_MODULE(FCourtOfSebelkehEditorModule, CourtOfSebelkehEditor);
DEFINE_LOG_CATEGORY(CourtOfSebelkehEditor);

#define LOCTEXT_NAMESPACE "CourtOfSebelkehEditor"
void FCourtOfSebelkehEditorModule::StartupModule()
{
    UE_LOG(CourtOfSebelkehEditor, Warning, TEXT("CourtOfSebelkehEditor: Log Started"));

    UThumbnailManager::Get().RegisterCustomRenderer(USkillBase::StaticClass(), USkillThumbnailRenderer::StaticClass());
    UThumbnailManager::Get().RegisterCustomRenderer(UBuff::StaticClass(), UBuffThumbnailRenderer::StaticClass());
}

void FCourtOfSebelkehEditorModule::ShutdownModule()
{
    UE_LOG(CourtOfSebelkehEditor, Warning, TEXT("CourtOfSebelkehEditor: Log Stopped"));
}

#undef LOCTEXT_NAMESPACE