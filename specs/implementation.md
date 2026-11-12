# Hud Marker Controls: behavior and implementation

Describes the retained payload **0.1.0**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.1.0** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.1.0** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.
<!-- /source-only-sync -->

## Behavior

H inside the native map opens marker controls. Keys 1–5 filter dust station, equipment, other, player and waypoint service markers; minus/plus cycle unlimited, 100, 250, 500 and 1,000 m; D toggles range labels. Exact native icon-array types are checked. CanvasGroup alpha filters front/rear service pins without replacing native marker layout.

## State and lifecycle

A checked world/player sidecar stores host preferences; guest edits remain session-only. Original marker alpha is captured and restored on teardown/world change. Filter state is separate from native landmark discovery and the native map remains responsible for marker data.

## Integration and configuration

No additional feature-mod runtime dependency is declared. Included shared headers are source reuse, not extra DLL dependencies.

The [module metadata](../source/mods/hud-marker-controls/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

## Implementation sources

- [mod.c](../source/mods/hud-marker-controls/mod.c)
- [runtime.h](../source/mods/hud-marker-controls/runtime.h)
- [store.h](../source/mods/hud-marker-controls/store.h)

## Verification boundary

Unrelated mod overlays and arbitrary map icons are outside this filter. Native service-kind matching, restored alpha, distance labels and guest behavior need game checks.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
