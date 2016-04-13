#pragma once
#include "Urho3DAll.h"

// Общее количество барьеров. Барьер - это пара труб (верхняя и нижняя).
// В процессе игры барьеры не создаются и не уничтожаются, а просто перемещаются
// в конец очереди, когда выходят из зоны видимости.
#define NUM_BARRIERS 3

// Зазор между трубами по вертикали. Через этот зазор должен пролетать НЛО.
#define BAR_GAP 10.0f

// Интервал между барьерами по горизонтали.
#define BAR_INTERVAL 20.0f

// Горизонтальная скорость движения барьеров.
#define BAR_SPEED 5.0f

// Случайное смещение барьера по высоте.
#define BAR_RANDOM_Y Random(-6.0f, 6.0f)

// Координата по горизонтали, при которой барьер гарантированно находится за пределами экрана.
#define BAR_OUTSIDE_X 30.0f

// Ускорение свободного падения.
#define GRAV_ACC 9.8f

// Скорость по вертикали, которую получает НЛО при прыжке.
#define UP_SPEED 10.0f

// Позиция камеры по умолчанию.
#define CAMERA_DEFAULT_POS Vector3(0.0f, 0.0f, -30.0f)

// НЛО повернуто боком к игроку.
#define UFO_DEFAULT_ROTATION Quaternion(0.0f, 90.0f, 0.0f)

// Макросы для более лаконичного кода.
#define CACHE GetSubsystem<ResourceCache>()
#define GLOBAL GetSubsystem<Global>()
#define INPUT GetSubsystem<Input>()
#define UI_ROOT GetSubsystem<UI>()->GetRoot()
#define RENDERER GetSubsystem<Renderer>()

// Состояния игры.
enum GameState
{
    // Заставка.
    // НЛО находится в центре координат, а игровой фон движется.
    // Таким образом создается иллюзия полета.
    // В этом режиме препятствия находятся за границей экрана и не двигаются.
    GS_INTRO = 0,

    // Игровой процесс.
    // НЛО находится в центре координат, игровой фон и препятствия движутся.
    GS_GAMEPLAY,

    // Игра закончена после столкновения с преградой.
    // Игровой фон и преграды не двигаются, а НЛО, кувыркаясь, улетает за пределы экрана.
    GS_DEAD
};

// Собственная подсистема для доступа к глобальным переменным и методам.
class Global : public Object
{
    URHO3D_OBJECT(Global, Object);

public:
    Global(Context* context);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    
    // Возвращает текущий счет.
    int GetScore() const { return score_; }

    // Устанавливает текущий счет и включает флаг необходимости
    // обновления текстового элемента со счетом на экране.
    void SetScore(int score);

    // Текущее состояние игры.
    GameState gameState_;

    // Требуемое состояние игры, которое будет установлено в начале следующего игрового цикла.
    GameState neededGameState_;

private:
    // Игровой счет.
    int score_;

    // Флаг необходимости обновления текстового элемента со счетом на экране.
    bool scoreTextDirty_;
};
