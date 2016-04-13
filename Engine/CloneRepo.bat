:: Указываем путь к git.exe
set "PATH=c:\Program Files (x86)\Git\bin\"
:: Скачиваем репозиторий
git clone https://github.com/Urho3D/Urho3D.git
:: Переходим в папку со скачанными исходниками
cd Urho3D
:: Возвращаем состояние репозитория к определённой версии (9 апреля 2016)
git reset --hard 4c8bd3efddf442cd31b49ce2c9a2e249a1f1d082
:: Ждём нажатия ENTER для закрытия консоли
pause
