#include "Result.h"
#include "DxLib.h"

namespace My3dApp
{
    Result::Result()
    {
        text = "4.Result";
    }

    Result::~Result()
    {
        // ?????Ȃ?
    }

    SceneBase* Result::Update(float deltaTime)
    {
        SceneBase* retScene = this;

        retScene = CheckRetScene(4);

        return retScene;
    }

    void Result::Draw()
    {
        CheckNowScene();
    }
}// namespace My3dApp