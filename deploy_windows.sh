#!/bin/bash

# --- НАСТРОЙКИ ПРОЕКТА ---
APP_NAME="FleetManager"
BUILD_DIR="build-win"
DIST_DIR="dist_windows"
MINGW_PATH="/usr/x86_64-w64-mingw32"
MINGW_BIN="$MINGW_PATH/bin"
QT_BIN="$MINGW_PATH/lib/qt6/bin"
QT_PLUGINS="$MINGW_PATH/lib/qt6/plugins"

# Переходим в директорию скрипта
pwd
cd "$(dirname "$0")"
pwd

echo "=== 1. Очистка старых сборок ==="
rm -rf $BUILD_DIR
rm -rf $DIST_DIR
mkdir $BUILD_DIR
mkdir $DIST_DIR

echo "=== 2. Кросс-компиляция приложения (CMake) ==="
cd $BUILD_DIR
x86_64-w64-mingw32-cmake ../src -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then echo "Ошибка CMake!"; exit 1; fi

make -j$(nproc)
if [ $? -ne 0 ]; then echo "Ошибка сборки!"; exit 1; fi

# Путь к EXE после сборки (обычно в папке src внутри build)
EXE_PATH="src/$APP_NAME.exe"
if [ ! -f "$EXE_PATH" ]; then
    # Если EXE лежит в корне билда
    EXE_PATH="$APP_NAME.exe"
fi

cp "$EXE_PATH" "../$DIST_DIR/"
cd ".."

echo "=== 3. Сбор библиотек и плагинов ==="
cd $DIST_DIR

# Функция для безопасного копирования
safe_cp() {
    if ls $1 >/dev/null 2>&1; then
        cp $1 .
        echo "OK: $(basename $1)"
    else
        echo "MISSING: $1"
    fi
}

# Основные библиотеки Qt6
safe_cp "$QT_BIN/Qt6Core.dll"
safe_cp "$QT_BIN/Qt6Gui.dll"
safe_cp "$QT_BIN/Qt6Widgets.dll"
safe_cp "$QT_BIN/Qt6Sql.dll"

# Рантайм MinGW
safe_cp "$MINGW_BIN/libgcc_s_seh-1.dll"
safe_cp "$MINGW_BIN/libstdc++-6.dll"
safe_cp "$MINGW_BIN/libwinpthread-1.dll"
safe_cp "$MINGW_BIN/libssp-0.dll"

# Системные зависимости (ICU, Fonts, Сжатие, SQL)
safe_cp "$MINGW_BIN/libiconv-2.dll"
safe_cp "$MINGW_BIN/libintl-8.dll"
safe_cp "$MINGW_BIN/libharfbuzz-0.dll"
safe_cp "$MINGW_BIN/libfreetype-6.dll"
safe_cp "$MINGW_BIN/libpng16-16.dll"
safe_cp "$MINGW_BIN/libgraphite2.dll"
safe_cp "$MINGW_BIN/libbrotlidec.dll"
safe_cp "$MINGW_BIN/libbrotlicommon.dll"
safe_cp "$MINGW_BIN/libpcre2-16-0.dll"
safe_cp "$MINGW_BIN/libpcre2-8-0.dll"
safe_cp "$MINGW_BIN/libglib-2.0-0.dll"
safe_cp "$MINGW_BIN/libzstd.dll"
safe_cp "$MINGW_BIN/zlib1.dll"
safe_cp "$MINGW_BIN/libbz2-1.dll"
safe_cp "$MINGW_BIN/libsqlite3-0.dll"

# Плагины (Windows Platform и SQL Driver)
mkdir platforms
cp "$QT_PLUGINS/platforms/qwindows.dll" platforms/
mkdir sqldrivers
cp "$QT_PLUGINS/sqldrivers/qsqlite.dll" sqldrivers/

echo "=== 4. Проверка через Wine ==="
if command -v wine &> /dev/null; then
    # Проверяем на ошибки импорта
    ERRORS=$(wine "$APP_NAME.exe" 2>&1 | grep "import_dll")
    if [ -z "$ERRORS" ]; then
        echo "Wine: Библиотеки найдены, приложение должно работать."
    else
        echo "Wine: Обнаружены ошибки!"
        echo "$ERRORS"
    fi
fi

echo "=== 5. Упаковка в ZIP ==="
# Мы используем ZIP, так как SFX на Linux создает несовместимые с Windows файлы
ZIP_NAME="../${APP_NAME}_Windows_Build.zip"
rm -f "$ZIP_NAME"
7z a "$ZIP_NAME" *

echo "========================================"
echo "ГОТОВО! Файл для друга: ${APP_NAME}_Windows_Build.zip"
echo "========================================"