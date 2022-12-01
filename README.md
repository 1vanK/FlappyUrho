*Более актуальная версия находится тут: <https://github.com/urho3d-learn/flappy-urho>.*

---

# Flappy Urho
Клон игры Flappy Bird для статьи https://habrahabr.ru/post/280752/.<br>
В папке Result есть скомпилированная версия.
## Управление
Левая кнопка мыши - прыгать<br>
Колесо мыши - отодвинуть/приблизить камеру (в целях отладки)<br>
Правая кнопка мыши - вернуть камеру на место<br>
Пробел - показать/спрятать отладочную геометрию

![Screenshot](https://raw.githubusercontent.com/1vanK/FlappyUrho/master/Screen.png)

P.S. В модели летающей тарелки 32 тысячи полигонов. По-хорошему нужно было создавать low-poly модель и запекать нормали. Но так как в сцене мало объектов, то я решил на это не заморачиваться.

Используемая версия движка указана в Engine/CloneRepo.bat. Помните, что если вы компилируете игру с более новым движком, вам нужно использовать CoreData от нового движка.
