#include "StageSelect.h"
#include "DxLib.h"

namespace My3dApp
{
    StageSelect::StageSelect()
    {
        text = "2.StageSelect";
    }

    StageSelect::~StageSelect()
    {
        // ?????Ȃ?
    }

    SceneBase* StageSelect::Update(float deltaTime)
    {
        SceneBase* retScene = this;

        retScene = CheckRetScene(2);

        return retScene;
    }

    void StageSelect::Draw()
    {
        CheckNowScene();
    }
}/** namespace My3dApp*/