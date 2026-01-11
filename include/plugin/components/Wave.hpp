#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

enum class WavePatternType : std::uint8_t
{
  POINT,
  LINE,
  CIRCLE,
  SPIRAL,
  GRID,
  FORMATION_V,
  ARC
};

inline const std::unordered_map<std::string, WavePatternType>
    WAVE_PATTERN_TYPE_MAP {{"point", WavePatternType::POINT},
                           {"line", WavePatternType::LINE},
                           {"circle", WavePatternType::CIRCLE},
                           {"spiral", WavePatternType::SPIRAL},
                           {"grid", WavePatternType::GRID},
                           {"formation_v", WavePatternType::FORMATION_V},
                           {"arc", WavePatternType::ARC}};

inline WavePatternType parse_wave_pattern_type(const std::string& str)
{
  auto it = WAVE_PATTERN_TYPE_MAP.find(str);
  if (it != WAVE_PATTERN_TYPE_MAP.end()) {
    return it->second;
  }
  return WavePatternType::POINT;
}

struct WavePattern
{
  WavePattern() = default;

  WavePattern(WavePatternType t, Vector2D orig, JsonObject p)
      : type(t)
      , origin(orig)
      , params(std::move(p))
  {
  }

  HOOKABLE(WavePattern, HOOK(type), HOOK(origin), HOOK(params))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(WavePattern,
                           ([](WavePatternType t, Vector2D orig, JsonObject p)
                            { return WavePattern {t, orig, std::move(p)}; }),
                           parseByte<WavePatternType>(),
                           parseVector2D(),
                           parseByteJsonObject())

  DEFAULT_SERIALIZE(type_to_byte(type),
                    vector2DToByte(origin),
                    json_object_to_byte(params))

  WavePatternType type = WavePatternType::POINT;
  Vector2D origin;
  JsonObject params;
};

inline Parser<WavePattern> parseWavePattern()
{
  return apply([](WavePatternType t, Vector2D orig, JsonObject p)
               { return WavePattern {t, orig, std::move(p)}; },
               parseByte<WavePatternType>(),
               parseVector2D(),
               parseByteJsonObject());
}

struct OnEndEvent
{
  OnEndEvent() = default;

  OnEndEvent(std::string e, JsonObject p)
      : event_name(std::move(e))
      , params(std::move(p))
  {
  }

  HOOKABLE(OnEndEvent, HOOK(event_name), HOOK(params))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      OnEndEvent,
      ([](std::string e, JsonObject p)
       { return OnEndEvent {std::move(e), std::move(p)}; }),
      parseByteString(),
      parseByteJsonObject())

  DEFAULT_SERIALIZE(string_to_byte(event_name), json_object_to_byte(params))

  std::string event_name;
  JsonObject params;
};

inline Parser<OnEndEvent> parseOnEndEvent()
{
  return apply([](std::string event_name, JsonObject p)
               { return OnEndEvent {std::move(event_name), std::move(p)}; },
               parseByteString(),
               parseByteJsonObject());
}

struct Wave
{
  Wave() = default;

  Wave(std::size_t wave_id,
       std::string tmpl,
       int cnt,
       WavePattern pat,
       OnEndEvent end,
       bool trk = true,
       bool has_spawned = false,
       std::vector<std::string> inherit = {})
      : id(wave_id)
      , entity_template(std::move(tmpl))
      , count(cnt)
      , pattern(std::move(pat))
      , on_end(std::move(end))
      , tracked(trk)
      , spawned(has_spawned)
      , components_inheritance(std::move(inherit))
  {
  }

  HOOKABLE(Wave,
           HOOK(id),
           HOOK(entity_template),
           HOOK(count),
           HOOK(pattern),
           HOOK(on_end),
           HOOK(tracked),
           HOOK(spawned),
           HOOK(components_inheritance))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(Wave,
                           (
                               [](std::size_t wave_id,
                                  std::string tmpl,
                                  int cnt,
                                  WavePattern pat,
                                  OnEndEvent end,
                                  bool trk,
                                  bool has_spawned,
                                  std::vector<std::string> inherit)
                               {
                                 return Wave {wave_id,
                                              std::move(tmpl),
                                              cnt,
                                              std::move(pat),
                                              std::move(end),
                                              trk,
                                              has_spawned,
                                              std::move(inherit)};
                               }),
                           parseByte<std::size_t>(),
                           parseByteString(),
                           parseByte<int>(),
                           parseWavePattern(),
                           parseOnEndEvent(),
                           parseByte<bool>(),
                           parseByte<bool>(),
                           parseByteArray(parseByteString()))

  DEFAULT_SERIALIZE(type_to_byte(id),
                    string_to_byte(entity_template),
                    type_to_byte(count),
                    pattern.to_bytes(),
                    on_end.to_bytes(),
                    type_to_byte(tracked),
                    type_to_byte(spawned),
                    vector_to_byte<std::string>(components_inheritance,
                                                string_to_byte))

  std::size_t id = 0;
  std::string entity_template;
  int count = 1;
  WavePattern pattern;
  OnEndEvent on_end;
  bool tracked = true;
  bool spawned = false;
  std::vector<std::string> components_inheritance;
};
