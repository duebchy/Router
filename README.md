# Route Finder

CLI-приложение на C++23 для поиска маршрутов через API Яндекс Расписаний.
Поддерживает прямые маршруты и маршруты с пересадками, интерактивный режим,
локальный справочник станций и LRU+TTL кэш запросов.

## Стек

- `C++23`, `CMake`
- `cpr`, `nlohmann/json`
- `GoogleTest`, `GoogleMock`
- `Docker`
- `std::expected`, `std::optional`, `std::chrono`, `std::filesystem`

## Архитектура

Код разделен на слои:

- `api` - клиент Яндекс Расписаний и HTTP-адаптер поверх `cpr`.
- `stations` - репозиторий кодов станций.
- `router` - построение маршрутов и парсинг ответа API.
- `cache` - in-memory LRU+TTL кэш.
- `bin` - REPL/CLI.
- `tests` - unit-тесты с моками.

Использованные паттерны:

- `Dependency Injection`: зависимости передаются через конструкторы.
- `Repository`: `StationRepository` скрывает источник кодов станций.
- `Decorator`: `CachedRouter` добавляет кэширование поверх `Router`.
- `Adapter`: `CprHttpSession` адаптирует `cpr` к `HttpSessionInterface`.

Основные интерфейсы:

- `ApiClientInterface`
- `HttpSessionInterface`
- `StationRepositoryInterface`
- `RouterInterface`

За счет интерфейсов бизнес-логика не зависит напрямую от HTTP-клиента и
файловой системы, а сетевые вызовы мокируются в тестах.

## Кэширование

Кэш реализован как `LRU + TTL`.

- `std::unordered_map` хранит записи и дает быстрый доступ по ключу.
- `std::list` хранит порядок использования ключей.
- `Get` переносит найденный ключ в начало списка.
- При переполнении удаляется последний элемент списка.
- Каждая запись хранит `expires_at`.
- Просроченная запись удаляется при чтении.
- Для будущих маршрутов TTL равен 6 часам.
- Для прошедших дат TTL равен 30 дням.

Ключ маршрута:

```text
from|to|date
```

Кэш подключен через `CachedRouter`, поэтому базовый `Router` не содержит логики
кэширования.

## API и переменные окружения

Используются endpoint'ы Яндекс Расписаний:

- `/v3.0/stations_list/`
- `/v3.0/search/`

Переменные:

```env
YANDEX_API_KEY=your_api_key
STATIONS_FILE=stations.json
```

При локальном запуске `.env` читается приложением. В Docker `.env` передается
через `--env-file`.

## Docker

Сборка:

```bash
docker build -t route-finder .
```

Запуск:

```bash
docker run --rm -it --env-file .env route-finder
```

Запуск с сохранением справочника станций:

```bash
docker run --rm -it --env-file .env -v route-finder-data:/data route-finder
```

В контейнере `STATIONS_FILE=/data/stations.json`.

## Использование

```text
Usage: <from> -> <to> <date>  (or 'history' / 'exit')
> Санкт-Петербург -> Псков 25.03.2026
> Москва -> Казань 2026-04-10
> history
> exit
```

Форматы даты: `DD.MM.YYYY`, `DD.MM.YY`, `YYYY-MM-DD`.

## Локальная сборка

```bash
cmake -S . -B build
cmake --build build
./build/bin/router
```

## Тесты

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Покрыты: API-клиент, маршрутизатор, `CachedRouter`, LRU/TTL кэш, `.env`,
репозиторий станций.
