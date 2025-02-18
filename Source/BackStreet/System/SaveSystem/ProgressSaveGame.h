#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/SaveGame.h"
#include "ProgressSaveGame.generated.h"

UCLASS()
class BACKSTREET_API UProgressSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // ������ ������

    // �⺻ ������
    UProgressSaveGame();

    UPROPERTY()
        bool bIsTutorialDone = false;
};
