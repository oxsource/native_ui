# Data Model: External Font Registration

**Feature**: `012-android-font-support` | **Phase 1 output of `/speckit.plan`**

## Entities & Relationships

```text
                    ┌────────────────────────────────────────────┐
                    │              FontManager (singleton)        │
                    │────────────────────────────────────────────│
                    │  registry_: map<FamilyName, WeightVariant[]>│
                    │  cache_:    map<FamilyKey, ResolvedFont>    │
                    │  default_family_ / default_entry_           │
                    │  platform_mgr_: <current FontMgr>           │
                    └───────┬───────────────────────┬────────────┘
                            │ 1                     │ 1
                            │ contains              │ retains
              ┌─────────────▼──────┐    ┌───────────▼──────────────┐
              │ RegisteredFontEntry│    │      ResolvedFont        │
              │────────────────────│    │──────────────────────────│
              │ family: string     │    │ family/weight key        │
              │ weight: int (100-900)│   │ path: string             │
              │ path: string       │    │ SkData (kept alive)      │
              │                    │ 1..N│ SkTypeface (sk_sp)      │
              └─────────────┬──────┘    └───────────▲──────────────┘
                            │ keyed by               │ produced from
                            │ (family, weight)        │
                            └────────────────────────┴────────────────────┐
                                                                         │
                                                        ┌────────────────▼─┐
                                                        │  Platform FontMgr│
                                                        │──────────────────│
                                                        │ CoreText (macOS) │
                                                        │ CustomDir /system│
                                                        │ /fonts (Android) │
                                                        │ CustomDir default│
                                                        │ dir (Linux)      │
                                                        └──────────────────┘
```

## Entity Definitions

### Font family name
- **Represents**: The developer-chosen identifier under which font files are registered and referenced by `FontFamily(...)`.
- **Attributes**: `string name`.
- **Uniqueness**: case-sensitive exact match; a family is unique across the registry (re-register overwrites).
- **Special value**: empty string `""` = "unset" → resolves to the default font (FR-013).

### RegisteredFontEntry (family, weight, path)
- **Represents**: one font file registered by the developer.
- **Attributes**: `family: string`, `weight: int` (100–900), `path: string` (abspath usable by the process).
- **Uniqueness**: (family, weight) is unique; registering the same (family, weight) with a new path replaces the entry (FR-010).
- **Relationship**: 1..N variants per family (FR-002); each family has exactly one "nearest-variant" resolution rule (FR-004).

### ResolvedFont
- **Represents**: the concrete font resource bound to a (family, weight) lookup.
- **Attributes**: reference to owning `RegisteredFontEntry`, cached `SkData` (kept alive for the typeface's backing bytes), cached `sk_sp<SkTypeface>` (framework-internal, never leaked to public API).
- **Lifecycle**: created lazily on first resolution; evicted when its family is re-registered or the default changes.

### Font resolution cache
- **Key**: `(family, weight)` — plus the default-font alias resolved to its own (family, weight).
- **Boundedness invariant**: `|cache| <= |registry|` (each registered variant maps to exactly one cache slot); repeated resolves hit the same slot → no growth over 10,000 resolutions (FR-011, SC-004).
- **Refresh rule**: re-registering `(family, weight)` clears that slot; the new path reloads on next resolve (FR-010).

### Default font
- **Represents**: the designated fallback target used when family is empty/unset, unknown, or the registered path failed (FR-013).
- **State**: implicit = first successfully registered variant; explicit = name set via `SetDefaultFont(family)` overriding the implicit choice (FR-014).
- **Constraints**: only a registered family can be designated; designating an unknown family is an error (no-op + observable error). Default designation persists across re-registration of that family's path.

## Validation Rules (mapped to FRs)

| Rule | FR |
|------|----|
| Register requires non-empty family and a path; weight clamped to 100–900 (out-of-range → clamp to nearest; default 400 when omitted). | FR-001/002 |
| Register on a path that is missing/unreadable/corrupt → observable error, entry NOT created, family falls back to default. | FR-006 |
| Referencing unknown/unregistered family → default font (if any), else platform default; must not crash. | FR-007 |
| Peer review note: measure a `Font{family,weight,size}` and draw it must resolve to the same `ResolvedFont`. | FR-008 |
| `FontFamily` via constructor tags and via `ApplyStyle(Style)` both land in `style_` → identical effect. | FR-009 |
| Re-register same (family, weight) → replace entry + evict cache slot. | FR-010 |
| Cache cardinality ≤ registry cardinality → memory bounded. | FR-011 |
| No registration at all → text path identical to today (platform default, e.g. CoreText on macOS). | FR-012 |
| First successful registration → becomes default; empty/unset/unknown/bad-file → default font. | FR-013 |
| `SetDefaultFont(family)` → repoint default; invalid family → observable error, default unchanged. | FR-014 |

## State Transitions

```text
(no default)
   │  first RegisterFont(...) success
   ▼
default_family_ = that family (implicit, FR-013)
   │
   │ SetDefaultFont(other_registered_family)            │ RegisterFont on default family (new path)
   ▼                                                  │ (refresh keeps designation, FR-010)
default_family_ = other (explicit, FR-014)             │
   ▲                                                  │
   └── (no reset path in v1; explicit override only)   ▼
                               default entry re-pointed to refreshed path
```

- Register of a *failed* path never sets/keeps default (FR-006).
- Rewriting the default family's path keeps the default designation but refreshes the resolved typeface (edge case #2 in spec).