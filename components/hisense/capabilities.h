#pragma once
// Unit identification and capabilities, ported from the stock AEH-W4G2 WiFi
// module firmware (Realtek RTL8710BN, build 2022-06-30).
//
// At start-up the stock firmware asks the indoor unit three read-only questions:
//   07 01 00  -> version reply      07 01 xx SW_hi SW_lo PROTO_hi PROTO_lo
//   0A 04 00  -> device type reply  0A 04 xx TYPE SUBTYPE
//   66 40 00  -> capability reply   66 40 xx <capability bits ...>
// The capability bits are reduced to a "product type" by a fixed decision
// function (0x806369c). The table below is that function, extracted by
// emulating it over every combination of the bits it reads and checked
// against it on 49,000 random and exhaustive inputs (0 mismatches).
// Device type 1 = split AC (products 100-113), 54 = 200-203, 55 = 300-311.
// Device type 21 (dehumidifier, 400-403) is not supported by this component.
//
// The product type and protocol version then gate behaviour in the stock
// firmware; the parts this component uses are in HisenseCapabilities below.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <initializer_list>

namespace esphome {
namespace hisense {
namespace ac {

struct ProductTypeRule { uint8_t device_type; uint16_t product_type; uint8_t n; uint8_t cond[9][3]; };
static const ProductTypeRule PRODUCT_TYPE_RULES[] = {
    {1, 100, 8, {{5, 7, 1}, {10, 6, 1}, {12, 3, 0}, {13, 0, 0}, {14, 6, 1}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 101, 8, {{5, 7, 0}, {10, 6, 1}, {12, 3, 0}, {13, 0, 0}, {14, 6, 1}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 102, 8, {{5, 7, 1}, {10, 6, 0}, {12, 3, 0}, {13, 0, 0}, {14, 6, 1}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 103, 8, {{5, 7, 0}, {10, 6, 0}, {12, 3, 0}, {13, 0, 0}, {14, 6, 1}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 104, 8, {{5, 7, 1}, {10, 6, 1}, {12, 3, 0}, {13, 0, 0}, {14, 6, 0}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 105, 8, {{5, 7, 0}, {10, 6, 1}, {12, 3, 0}, {13, 0, 0}, {14, 6, 0}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 106, 8, {{5, 7, 1}, {10, 6, 0}, {12, 3, 0}, {13, 0, 0}, {14, 6, 0}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 107, 8, {{5, 7, 0}, {10, 6, 0}, {12, 3, 0}, {13, 0, 0}, {14, 6, 0}, {15, 4, 0}, {15, 6, 0}, {19, 0, 0}}},
    {1, 108, 9, {{5, 7, 0}, {10, 6, 1}, {12, 3, 0}, {14, 6, 0}, {14, 7, 0}, {15, 4, 1}, {15, 6, 0}, {19, 0, 0}, {13, 1, 0}}},
    {1, 109, 9, {{5, 7, 1}, {10, 6, 1}, {12, 3, 0}, {14, 6, 0}, {14, 7, 0}, {15, 4, 1}, {15, 6, 1}, {19, 0, 0}, {13, 1, 0}}},
    {1, 110, 9, {{5, 7, 0}, {10, 6, 1}, {12, 3, 0}, {14, 6, 0}, {14, 7, 0}, {15, 4, 1}, {15, 6, 1}, {19, 0, 0}, {13, 1, 1}}},
    {1, 111, 9, {{5, 7, 1}, {10, 6, 1}, {12, 3, 0}, {14, 6, 0}, {14, 7, 0}, {15, 4, 1}, {15, 6, 1}, {19, 0, 0}, {13, 1, 1}}},
    {1, 112, 9, {{5, 7, 0}, {10, 6, 1}, {12, 3, 1}, {14, 6, 0}, {14, 7, 0}, {15, 4, 1}, {15, 6, 1}, {19, 0, 0}, {13, 1, 1}}},
    {1, 113, 9, {{5, 7, 1}, {10, 6, 1}, {12, 3, 1}, {14, 6, 0}, {14, 7, 0}, {15, 4, 1}, {15, 6, 1}, {19, 0, 0}, {13, 1, 1}}},
    {54, 200, 2, {{5, 7, 0}, {14, 6, 0}}},
    {54, 201, 2, {{5, 7, 1}, {14, 6, 0}}},
    {54, 202, 2, {{5, 7, 0}, {14, 6, 1}}},
    {54, 203, 2, {{5, 7, 1}, {14, 6, 1}}},
    {55, 300, 4, {{5, 7, 0}, {11, 6, 1}, {14, 6, 1}, {22, 0, 0}}},
    {55, 300, 4, {{5, 7, 0}, {11, 6, 1}, {14, 6, 1}, {22, 1, 0}}},
    {55, 301, 3, {{5, 7, 0}, {11, 6, 0}, {14, 6, 0}}},
    {55, 302, 4, {{5, 7, 1}, {11, 6, 1}, {14, 6, 1}, {22, 0, 0}}},
    {55, 302, 4, {{5, 7, 1}, {11, 6, 1}, {14, 6, 1}, {22, 1, 0}}},
    {55, 302, 4, {{5, 7, 1}, {11, 6, 1}, {12, 3, 1}, {14, 6, 1}}},
    {55, 303, 3, {{5, 7, 1}, {11, 6, 0}, {14, 6, 0}}},
    {55, 304, 6, {{5, 7, 0}, {11, 6, 0}, {12, 3, 0}, {14, 6, 1}, {22, 0, 1}, {22, 1, 1}}},
    {55, 305, 6, {{5, 7, 0}, {11, 6, 0}, {12, 3, 1}, {14, 6, 1}, {22, 0, 1}, {22, 1, 1}}},
    {55, 306, 6, {{5, 7, 0}, {11, 6, 1}, {12, 3, 0}, {14, 6, 1}, {22, 0, 1}, {22, 1, 1}}},
    {55, 307, 6, {{5, 7, 0}, {11, 6, 1}, {12, 3, 1}, {14, 6, 1}, {22, 0, 1}, {22, 1, 1}}},
    {55, 308, 6, {{5, 7, 1}, {11, 6, 1}, {12, 3, 0}, {14, 6, 1}, {22, 0, 1}, {22, 1, 1}}},
    {55, 309, 6, {{5, 7, 1}, {11, 6, 0}, {12, 3, 0}, {14, 6, 1}, {22, 0, 1}, {22, 1, 1}}},
    {55, 310, 6, {{5, 7, 0}, {11, 6, 0}, {12, 3, 0}, {14, 6, 1}, {22, 0, 0}, {22, 1, 0}}},
    {55, 311, 6, {{5, 7, 1}, {11, 6, 0}, {12, 3, 0}, {14, 6, 1}, {22, 0, 0}, {22, 1, 0}}},
};

/// cap = reply payload starting at the 0x66 command byte. Returns -1 if unknown.
inline int decode_product_type(uint8_t device_type, const uint8_t *cap, size_t len) {
  for (const auto &r : PRODUCT_TYPE_RULES) {
    if (r.device_type != device_type)
      continue;
    bool ok = true;
    for (uint8_t i = 0; i < r.n && ok; i++) {
      const uint8_t idx = r.cond[i][0];
      ok = idx < len && ((cap[idx] >> r.cond[i][1]) & 1) == r.cond[i][2];
    }
    if (ok)
      return r.product_type;
  }
  return -1;
}

inline bool product_in(int product, std::initializer_list<int> list) {
  for (int p : list)
    if (p == product)
      return true;
  return false;
}

// Feature bits of the capability reply (66 40), counted MSB first from its first
// data byte (frame byte 16), as in straga's xm_KTGN table. Single-bit flags only;
// the multi-bit counts are decoded in HisenseCapabilities.
struct CapabilityFeature {
  uint8_t bit;
  const char *name;
};
static const CapabilityFeature CAPABILITY_FEATURES[] = {
    {16, "heat pump"},       {17, "power"},          {18, "single vane"},     {19, "setpoint"},
    {20, "humidity setpoint"}, {21, "feel temp"},     {22, "feel comp"},       {23, "auto/dry comp"},
    {40, "F/C"},             {41, "countdown timer"}, {42, "clock timers"},    {43, "6-pos vane"},
    {44, "up/down swing"},   {45, "left/right swing"}, {46, "natural wind"},   {47, "electric heat"},
    {56, "energy saving"},   {57, "eco"},            {58, "turbo"},           {59, "dual mode"},
    {60, "fresh air"},       {61, "air exchange"},   {62, "indoor clean"},    {63, "outdoor clean"},
    {64, "smart eye"},       {65, "quiet"},          {66, "voice"},           {67, "smoke removal"},
    {68, "backlight"},       {69, "display glow"},   {70, "LED"},             {71, "in/out temp display"},
    {72, "filter reset"},    {73, "left vane"},      {74, "right vane"},      {79, "VOC"},
    {80, "power saver"},
};

struct HisenseCapabilities {
  uint8_t device_type{0};   // 0 = unknown
  uint8_t device_subtype{0};
  int product_type{-1};     // -1 = unknown
  uint16_t software_version{0};
  uint16_t protocol_version{0};  // 0 = unknown
  // Capability reply data bytes (from frame byte 16), straga XM_KT_SUPPORT_LEN = 17.
  static const uint8_t SUPPORT_LEN = 17;
  uint8_t support[SUPPORT_LEN]{};
  uint8_t support_len{0};  // 0 = no capability reply

  /// Capability bit (MSB first from the first data byte).
  bool has(uint8_t bit) const {
    const uint8_t i = bit / 8;
    return i < support_len && ((support[i] >> (7 - bit % 8)) & 1);
  }
  uint8_t support_byte(uint8_t i) const { return i < support_len ? support[i] : 0; }
  uint8_t fan_levels() const { return support_byte(0); }
  uint8_t sleep_profiles() const { return support_byte(1); }
  uint8_t feel_compensation_range() const { return support_byte(3); }
  uint8_t auto_dry_compensation_range() const { return support_byte(4); }
  uint8_t dry_modes() const { return support_byte(6); }
  /// Short summary: counts, then the supported feature flags.
  std::string summary() const {
    if (support_len == 0)
      return "unknown";
    char buf[96];
    snprintf(buf, sizeof(buf), "fan %u, sleep %u, dry %u, feel +-%u, comp +-%u", fan_levels(), sleep_profiles(),
             dry_modes(), feel_compensation_range(), auto_dry_compensation_range());
    std::string s = buf;
    for (const auto &f : CAPABILITY_FEATURES) {
      if (has(f.bit)) {
        s += ", ";
        s += f.name;
      }
    }
    return s;
  }

  bool is_supported_device() const {
    return device_type == 0 || device_type == 1 || device_type == 54 || device_type == 55;
  }
  /// Stock firmware: on split units (type 1) with protocol <= 532, or products
  /// 102/103/106/107, the fan has 3 speeds sent as 2/3/4 instead of 5..9.
  bool three_speed_fan() const {
    if (device_type != 1)
      return false;
    if (protocol_version != 0 && protocol_version <= 532)
      return true;
    return product_in(product_type, {102, 103, 106, 107});
  }
  /// Stock firmware ("f_ecm"): protocol >= 534 reports power as a 16-bit value in
  /// status bytes 142-143; older units use bytes 53-55.
  bool power_in_extended_frame() const { return protocol_version == 0 || protocol_version >= 534; }
  /// Products that the stock firmware reports continuous fan % for (t_fanspeedcv).
  bool fan_percent() const { return product_in(product_type, {112, 113, 118, 119}); }
  /// Products the stock firmware reports louver direction/angle for.
  bool swing_direction() const { return product_type >= 108 && product_type <= 119; }
  /// Products the stock firmware does NOT report power/voltage to the cloud for.
  bool power_not_reported() const {
    return product_in(product_type, {104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 200, 201, 301, 303, 400, 401});
  }
};

}  // namespace ac
}  // namespace hisense
}  // namespace esphome
