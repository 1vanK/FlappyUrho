#include "Global.h"
#include "BarrierLogic.h"
#include "UfoLogic.h"
#include "EnvironmentLogic.h"
#include "CameraLogic.h"

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

private:
    // Указатель на сцену.
    Scene* scene_;

    // Флаг, указывающий, нужно ли рендерить отладочную геометрию.
    bool drawDebug_;

public:
    Game(Context* context) :
        Application(context),
        scene_(nullptr),
        drawDebug_(false)
    {
        // Обработчики событий вызываются в порядке их подписания на события,
        // поэтому метод, в котором будет производиться смена состояния игры,
        // первым привязываем к самому первому событию.
        SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(Game, HandleBeginFrame));

        // Регистрируем логические компоненты.
        BarrierLogic::RegisterObject(context);
        UfoLogic::RegisterObject(context);
        EnvironmentLogic::RegisterObject(context);
        CameraLogic::RegisterObject(context);

        // Регистрируем собственную подсистему.
        context->RegisterSubsystem(new Global(context));
    }

    void Setup()
    {
        // Указываем движку папки с ресурсами.
        engineParameters_["ResourcePaths"] = "Data;CoreData;GameData";
    }

    void Start()
    {
        // Инициализируем генератор случайных чисел текущим времнем.
        // Каждая игра будет уникальной.
        SetRandomSeed(Time::GetSystemTime());

        CreateScene();
        CreateUfo();
        CreateBarriers();
        CreateUI();

        // Указываем движку какая камера какой сцены будет показываться на экране.
        auto camera = scene_->GetChild("Camera")->GetComponent<Camera>();
        auto viewport = new Viewport(context_, scene_, camera);
        RENDERER->SetViewport(0, viewport);

        // Добавляем сглаживание.
        viewport->GetRenderPath()->Append(CACHE->GetResource<XMLFile>("PostProcess/FXAA3.xml"));

        // Подписываемся на остальные события.
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Game, HandlePostRenderUpdate));
    }

    // Изменяем состояние игры только в начале игрового цикла.
    // Подробное пояснение смотрите в статье на Хабре.
    void HandleBeginFrame(StringHash eventType, VariantMap& eventData)
    {
        if (GLOBAL->gameState_ == GLOBAL->neededGameState_)
            return;

        // В этом месте мы анализируем из какого и в какое состояние хочет перейти
        // игра и выполняем нужные действия при этом.

        // Если НЛО врезалось в преграду,
        if (GLOBAL->neededGameState_ == GS_DEAD)
        {
            // то воспроизводим звук взрыва.
            auto ufoNode = scene_->GetChild("Ufo");
            auto soundSource = ufoNode->GetOrCreateComponent<SoundSource>();
            soundSource->Play(CACHE->GetResource<Sound>("Sounds/Explosion.ogg"));
        }
        // Если игра переходит в режим заставки (из режима GS_DEAD),
        else if (GLOBAL->neededGameState_ == GS_INTRO)
        {
            // то возвращаем НЛО в исходное состояние,
            auto ufoNode = scene_->GetChild("Ufo");
            ufoNode->SetPosition(Vector3::ZERO);
            ufoNode->SetRotation(UFO_DEFAULT_ROTATION);
            auto ufoLogic = ufoNode->GetComponent<UfoLogic>();
            ufoLogic->Reset();

            // сбрасываем счётчик очков
            GLOBAL->SetScore(0);

            // и перестраиваем уровень.
            PODVector<Node*> barriers;
            scene_->GetChildrenWithComponent<BarrierLogic>(barriers);
            for (unsigned i = 0; i < barriers.Size(); i++)
            {
                auto pos = barriers[i]->GetPosition();
                pos.y_ = BAR_RANDOM_Y;

                // Если барьер находится позади НЛО или расположен слишком
                // близко к тарелке (виден на экране), то перемещаем его в конец очереди.
                if (pos.x_ < BAR_OUTSIDE_X)
                    pos.x_ += NUM_BARRIERS * BAR_INTERVAL;

                barriers[i]->SetPosition(pos);
            }
        }

        // Наконец меняем игровое состояние.
        GLOBAL->gameState_ = GLOBAL->neededGameState_;

        // Обновляем видимость элементов интерфейса.
        UpdateUIVisibility();
    }

    // Показываем/прячем элементы интерфейса в зависимости от состояния игры.
    // Так как пользовательский интерфейс один на всю игру, то при переходе игры из одного
    // состояния в другое нужно обладать информацией, какие элементы должны быть на экране.
    // Для этой цели удобно использовать теги.
    void UpdateUIVisibility()
    {
        String tag;
        if (GLOBAL->gameState_ == GS_GAMEPLAY)     tag = "Gameplay";
        else if (GLOBAL->gameState_ == GS_DEAD)    tag = "Dead";
        else                                       tag = "Intro";

        // Цикл по всем элементам интерфейса.
        auto uiElements = UI_ROOT->GetChildren();
        for (auto i = uiElements.Begin(); i != uiElements.End(); i++)
        {
            auto element = *i;
            element->SetVisible(element->HasTag(tag));
        }
    }

    // Создаем игровой интерфейс.
    void CreateUI()
    {
        auto font = CACHE->GetResource<Font>("Fonts/Ubuntu-BI.ttf");

        // Создаем текстовый элемент для вывода счёта.
        auto scoreText = UI_ROOT->CreateChild<Text>("Score");
        scoreText->SetFont(font, 40);
        scoreText->SetTextEffect(TE_STROKE);
        scoreText->SetColor(Color::BLACK);
        scoreText->SetEffectColor(Color::WHITE);
        
        // Изначально текстовый элемент со счётом не виден на экране.
        scoreText->SetVisible(false);
        
        // Используем теги для указания того, в каких состояниях игры будет виден элемент.
        scoreText->AddTags("Gameplay;Dead");

        // Создаем текстовый элемент для вывода подсказки.
        auto helpText = UI_ROOT->CreateChild<Text>();
        helpText->SetFont(font, 20);
        helpText->SetTextEffect(TE_SHADOW);
        helpText->SetAlignment(HA_CENTER, VA_CENTER);
        helpText->SetPosition(0, UI_ROOT->GetHeight() / 4);
        helpText->AddTags("Intro;Dead");

        // Так как данный исходник сохранен в кодировке UTF-8 с маркером BOM (в отличие от Global.cpp),
        // то необходимо задавать строку в формате wchar_t* (по крайней мере для компилятора VC).
        helpText->SetText(L"Жмите левую кнопку мыши");
    }

    void CreateScene()
    {
        scene_ = new Scene(context_);
        scene_->CreateComponent<Octree>();
        scene_->CreateComponent<DebugRenderer>();

        // Компонент PhysicsWorld можно и не создавать,
        // так как он создается автоматически при необходимости.
        scene_->CreateComponent<PhysicsWorld>();

        // Создаем ноду и прикрепляем к ней камеру и управляющий логический компонент.
        auto cameraNode = scene_->CreateChild("Camera");
        auto camera = cameraNode->CreateComponent<Camera>();
        cameraNode->SetPosition(CAMERA_DEFAULT_POS);
        cameraNode->CreateComponent<CameraLogic>();

        // Создаем ноду и прикрепляем к ней источник света (солнечный свет).
        auto lightNode = scene_->CreateChild();
        auto light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        lightNode->SetDirection(Vector3(-0.5f, -1.0f, 1.0f));

        // Создаем ноду и прикрепляем к ней скайбокс и управляющий логический компонент.
        auto envNode = scene_->CreateChild();
        auto skybox = envNode->CreateComponent<Skybox>();
        skybox->SetModel(CACHE->GetResource<Model>("Models/Box.mdl"));
        skybox->SetMaterial(CACHE->GetResource<Material>("Materials/Env.xml"));
        envNode->CreateComponent<EnvironmentLogic>();
    }

    // Добавляем в сцену НЛО.
    void CreateUfo()
    {
        auto ufoNode = scene_->CreateChild("Ufo");
        auto ufoObject = ufoNode->CreateComponent<AnimatedModel>();
        ufoObject->SetModel(CACHE->GetResource<Model>("Models/Ufo.mdl"));
        ufoObject->SetCastShadows(true);
        ufoNode->SetRotation(UFO_DEFAULT_ROTATION);
        ufoNode->CreateComponent<UfoLogic>();

        // Так как модель состоит из нескольких материалов,
        // то список материалов удобно загружать из файла (Models/Ufo.txt).
        ufoObject->ApplyMaterialList();

        // Запускаем циклическое проигрывание анимации (покачивание НЛО).
        auto animCtrl = ufoNode->CreateComponent<AnimationController>();
        animCtrl->PlayExclusive("Models/Fly.ani", 0, true);

        auto body = ufoNode->CreateComponent<RigidBody>();
        // Два статических тела не будут генерировать события о столкновениях,
        // поэтому хотя бы одно тело должно быть динамическим.
        // Задаем для НЛО массу больше нуля, чтобы сделать его динамическим.
        body->SetMass(1.0f);
        // Но нам не нужна симуляция физики, нас интересует только факт касания объектов.
        body->SetKinematic(true);

        // Нода с НЛО содержит несколько компонентов CollisionShape для более точного описания формы.
        // Размеры шейпов были подобраны в редакторе Urho3D.
        
        // Корпус.
        auto shape1 = ufoNode->CreateComponent<CollisionShape>();
        shape1->SetShapeType(SHAPE_CAPSULE);
        shape1->SetSize(Vector3(1.9f, 6.4f, 0.0f));
        shape1->SetPosition(Vector3(0.0f, -0.3f, 0.0f));
        shape1->SetRotation(Quaternion(90.f, 0.0f, 0.0f));

        // Кабина.
        auto shape2 = ufoNode->CreateComponent<CollisionShape>();
        shape2->SetShapeType(SHAPE_CAPSULE);
        shape2->SetSize(Vector3(1.6f, 3.3f, 0.0f));
        shape2->SetPosition(Vector3(0.0f, 0.8f, 0.0f));
        shape2->SetRotation(Quaternion(90.f, 0.0f, 0.0f));

        // Прикрепляем к НЛО источник голубого света.
        auto lightNode = ufoNode->CreateChild();
        lightNode->SetPosition(Vector3(0.0f, -3.0f, 0.0f));
        auto light = lightNode->CreateComponent<Light>();
        light->SetColor(Color(0.0f, 1.0f, 1.0f));
        light->SetSpecularIntensity(0.0f);
        light->SetRange(5.0f);
    }

    // Создание барьеров.
    // Барьер имеет две дочернии ноды - верхнюю и нижнюю трубы.
    // Зазор между трубами является физическим телом, чтобы можно было легко определить,
    // что НЛО успешно преодолел барьер и следует увеличить счетчик очков.
    void CreateBarriers()
    {
        for (int i = 0; i < NUM_BARRIERS; i++)
        {
            auto barrierNode = scene_->CreateChild("Barrier");
            barrierNode->CreateComponent<BarrierLogic>();

            // Барьеры размещаются за пределами экрана со случайным смещением по высоте.
            barrierNode->SetPosition(Vector3(BAR_OUTSIDE_X + i * BAR_INTERVAL, BAR_RANDOM_Y, 0.0f));

            // Создаём статическое физическое тело для зазора (размер формы столкновений
            // подбирался в редакторе Urho3D).
            barrierNode->CreateComponent<RigidBody>();
            auto shape = barrierNode->CreateComponent<CollisionShape>();
            shape->SetShapeType(SHAPE_BOX);
            shape->SetSize(Vector3(7.8f, BAR_GAP, 7.8f));

            // Создаем верхнюю и нижнюю трубы.
            CreatePipe(barrierNode, true);
            CreatePipe(barrierNode, false);
        }
    }

    // Создание ноды с трубой (верхней или нижней). Это дочерняя нода для барьера.
    Node* CreatePipe(Node* barrierNode, bool top)
    {
        auto pipeNode = barrierNode->CreateChild("Pipe");
        
        // Прикрепляем к ноде модель трубы.
        auto staticModel = pipeNode->CreateComponent<StaticModel>();
        staticModel->SetModel(CACHE->GetResource<Model>("Models/Pipe.mdl"));
        staticModel->SetCastShadows(true);
        staticModel->ApplyMaterialList();

        // Создаём статическое физическое тело для трубы (размер формы столкновений
        // подбирался в редакторе Urho3D).
        auto body = pipeNode->CreateComponent<RigidBody>();
        auto shape = pipeNode->CreateComponent<CollisionShape>();
        shape->SetShapeType(SHAPE_BOX);
        shape->SetSize(Vector3(7.8f, 30.0f, 7.8f));
        shape->SetPosition(Vector3(0.0f, -15.0f, 0.0f));

        // Если это верхняя труба,
        if (top)
        {
            // то смещаем ее вверх от центра барьера и переворчиваем.
            pipeNode->SetPosition(Vector3(0.0f, BAR_GAP / 2, 0.0f));
            pipeNode->SetRotation(Quaternion(180.0f, 0.0f, 0.0f));
        }
        else
        {
            // Иначе смещаем трубу вниз от центра барьера.
            pipeNode->SetPosition(Vector3(0.0f, -BAR_GAP / 2, 0.0f));
        }

        return pipeNode;
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;
        float timeStep = eventData[P_TIMESTEP].GetFloat();

        // Когда игрок нажимает левую кнопку мыши,
        if (INPUT->GetMouseButtonPress(MOUSEB_LEFT))
        {
            // если показывается заставка,
            if (GLOBAL->gameState_ == GS_INTRO)
                // то в начале следующего игрового цикла начнется игра.
                GLOBAL->neededGameState_ = GS_GAMEPLAY;
            // А если игра проиграна,
            else if (GLOBAL->gameState_ == GS_DEAD)
                // то в начале следующего игрового цикла будет показана заставка.
                GLOBAL->neededGameState_ = GS_INTRO;

            // Подпрыгивание НЛО при нажатии левой кнопки мыши реализовано
            // в другом месте (в компоненте UfoLogic).
        }

        // При нажатии пробела показываем/прячем отладочную геометрию.
        if (INPUT->GetKeyPress(KEY_SPACE))
            drawDebug_ = !drawDebug_;

        // При нажатии Esc выходим из игры.
        if (INPUT->GetKeyPress(KEY_ESC))
            engine_->Exit();
    }

    // После рендеринга сцены рисуем отладочную геометрию, если нужно.
    void Game::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
    {
        if (drawDebug_)
        {
            // Рендерим геометрические формы, которые используются для
            // расчета столкновений физических тел.
            scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Game)
