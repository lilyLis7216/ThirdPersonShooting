#include "Player.h"
#include "../Manager/AssetManager.h"
#include "../Library/Calc3D.h"
#include "../Library/AnimationController.h"

namespace My3dApp
{
    Player::Player()
        : GameObject(ObjectTag::Player)
        , animCtrl(nullptr)
        , animTypeID(0)
        , isRotate(false)
        , padInputState(0)
        , stickX(0)
        , stickY(0)
    {

        /** 3Dモデルの読み込み*/
        modelHandle = AssetManager::GetMesh("../Assets/model/player2/unityChanModel.mv1");

        MV1SetScale(modelHandle, VGet(0.01f, 0.01f, 0.01f));

        /** アニメーションコントローラの生成*/
        animCtrl = new AnimationController(modelHandle);

        /** 待機アニメーションの読み込み*/
        animCtrl->AddAnimation("../Assets/model/player2/unityChanAnimIdle.mv1");

        /** 走行アニメーションの読み込み*/
        animCtrl->AddAnimation("../Assets/model/player2/unityChanAnimRun.mv1");

        /** 攻撃アニメーションの読み込み*/
        animCtrl->AddAnimation("../Assets/model/player2/unityChanAnimPunch.mv1");

        /** 初期再生アニメーションの初期化*/
        animCtrl->StartAnimation(animTypeID);

        /** 座標の初期化*/
        pos = VGet(0, 0, 0);

        /** 向きの初期化*/
        dir = VGet(1, 0, 0);

        /**　*/
        aimDir = dir;

        /** 速度の初期化*/
        speed = VGet(0, 0, 0);

        collisionType = CollisionType::Sphere;

        collisionSphere.localCenter = VGet(0, 50.0f, 0);

        collisionSphere.radius = 30.0f;

        collisionLine = LineSegment(VGet(0.0f, 20.0f, 0.0f), VGet(0.0f, -30.0f, 0.0f));
    }

    Player::~Player()
    {
        /** モデルのアンロード*/
        AssetManager::ReleaseMesh(modelHandle);

        /** アニメーションコントローラの削除*/
        delete animCtrl;
    }

    void Player::Update(float deltaTime)
    {
        animCtrl->AddAnimationTime(deltaTime);

        RotateCheck();

        InputCheck();

        Move(deltaTime);

        CollisionUpdate();
    }

    void Player::Draw()
    {
        /** 3Dモデルの描画*/
        MV1DrawModel(modelHandle);

        DrawCollider();
    }

    void Player::OnCollisionEnter(const GameObject* other)
    {
        ObjectTag tag = other->GetTag();

        /** マップとの衝突*/
        if (tag == ObjectTag::Map)
        {
            int collModel = other->GetCollisionModel();

            /** マップと自身の境界球との当たり判定*/
            MV1_COLL_RESULT_POLY_DIM collInfo;

            /** 当たっている場合*/
            if (CollisionPair(collisionSphere, collModel, collInfo))
            {
                /** 押し戻し量*/
                VECTOR pushBackVec = CalcSpherePushBackVecFromMesh(collisionSphere, collInfo);

                /** 押し戻し*/
                pos += pushBackVec;

                /** 当たり判定情報の解放*/
                MV1CollResultPolyDimTerminate(collInfo);

                /** 当たり判定の更新*/
                CollisionUpdate();
            }

            /** 背景と足元線分当たり判定*/
            MV1_COLL_RESULT_POLY collInfoLine;

            /** 当たっている場合*/
            if (CollisionPair(collisionLine, collModel, collInfoLine))
            {
                /** 足元を衝突点に合わせる*/
                pos = collInfoLine.HitPosition;

                /** 当たり判定の更新*/
                CollisionUpdate();
            }
        }
    }

    void Player::InputCheck()
    {
        /** パッドの入力を取得*/
        padInputState = GetJoypadInputState(DX_INPUT_KEY_PAD1);

        /** アナログスティックの入力を取得*/
        GetJoypadAnalogInput(&stickX, &stickY, DX_INPUT_KEY_PAD1);
    }

    void Player::Move(float deltaTime)
    {
        /** 左方向*/
        VECTOR LEFT = { -1, 0, 0 };

        /** 右方向*/
        VECTOR RIGHT = { 1, 0, 0 };

        /** 上方向*/
        VECTOR UP = { 0, 0, 1 };

        /** 下方向*/
        VECTOR DOWN = { 0, 0, -1 };

        /** 移動用ベクトル*/
        VECTOR inputVec = VGet(0, 0, 0);

        /** 入力があったかどうか*/
        bool input = false;

        if (0 < stickX || padInputState & PAD_INPUT_RIGHT)
        {
            inputVec += RIGHT;
            input = true;
        }

        if (stickX < 0 || padInputState & PAD_INPUT_LEFT)
        {
            inputVec += LEFT;
            input = true;
        }

        if (0 < stickY || padInputState & PAD_INPUT_DOWN)
        {
            inputVec += DOWN;
            input = true;
        }

        if (stickY < 0 || padInputState & PAD_INPUT_UP)
        {
            inputVec += UP;
            input = true;
        }

        /** 入力があったら*/
        if (input)
        {
            /** 左右上下同時押しなどで入力ベクトルが0のとき*/
            if (VSquareSize(inputVec) < 0.5f)
            {
                return;
            }

            /** 方向を正規化*/
            inputVec = VNorm(inputVec);

            /** 入力方向は現在向いている方向と異なるか？*/
            if (IsNearAngle(inputVec, dir))
            {
                dir = inputVec;
            }
            else
            {
                isRotate = true;
                aimDir = inputVec;
            }

            speed = inputVec + (inputVec * deltaTime * 200.0f);

            /** もし他のモーション中だったら走りモーションへ*/
            if (animTypeID != 1)
            {
                animTypeID = 1;
                animCtrl->StartAnimation(animTypeID);
            }
        }
        else
        {
            speed *= 0.9f;

            /** もしほかのモーション中だったら歩きモーションへ*/
            if (animTypeID != 0)
            {
                animTypeID = 0;
                animCtrl->StartAnimation(animTypeID);
            }
        }

        pos += speed;

        /** 3Dモデルのポジション設定*/
        MV1SetPosition(modelHandle, pos);

        /** 向きに合わせてモデルを回転*/
        MATRIX rotYMat = MGetRotY(180.0f * (float)(DX_PI / 180.0f));
        VECTOR negativeVec = VTransform(dir, rotYMat);

        /** モデルに回転をセットする*/
        MV1SetRotationZYAxis(modelHandle, negativeVec, VGet(0.0f, 1.0f, 0.0f), 0.0f);

        /** 当たり判定モデルの位置更新*/
        CollisionUpdate();
    }

    void Player::RotateCheck()
    {
        if (isRotate)
        {
            /** 回転が目標角度に十分近ければ回転終了*/
            if (IsNearAngle(aimDir, dir))
            {
                dir = aimDir;
                isRotate = false;
            }
            else
            {
                /** 回転させる*/
                VECTOR interPolateDir = RotateForAimVecYAxis(dir, aimDir, 10.0f);

                /** 回転が目標角度を超えていないか*/
                VECTOR cross1 = VCross(dir, aimDir);
                VECTOR cross2 = VCross(interPolateDir, aimDir);

                /** 目標角度を超えたら終了*/
                if (cross1.y * cross2.y < 0.0f)
                {
                    interPolateDir = aimDir;
                    isRotate = false;
                }

                /** 目標ベクトルに10度だけ近づけた角度*/
                dir = interPolateDir;
            }
        }
    }
}/** namespace My3dApp*/