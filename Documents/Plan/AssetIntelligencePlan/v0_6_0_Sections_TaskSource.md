# AssetDump v0.6.0 Sections Option Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.6.0
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- implementation_status: completed
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_0_Sections_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- section_registry: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- result_log: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md`

## Goal

Add a `-Sections=` option to AssetDump so AI workflows can request only selected major JSON output sections.

The first implementation should be backward compatible. When `-Sections=` is omitted, AssetDump must keep the existing full dump behavior. When `-Sections=` is provided, AssetDump should emit only the requested major sections plus any minimal envelope fields required to identify the asset and dump status.

## In Scope

1. Add commandlet argument parsing for `-Sections=`.
2. Define a section selection data structure or helper that can answer whether a major section is enabled.
3. Support at minimum these section names:
   - `summary`
   - `digest`
   - `details`
   - `graphs`
   - `references`
   - `widget_designer`
4. Preserve current default behavior when `-Sections=` is not specified.
5. Apply section filtering during JSON serialization.
6. Ensure `widget_designer` output is emitted only when full mode is active or `widget_designer` is explicitly requested.
7. Keep minimal envelope fields required for asset identification and dump validity.
8. Add clear validation or smoke test coverage for selected section output.
9. Add clear failure behavior for unknown section names.
10. Document migration behavior in changelog comments or implementation notes if existing command behavior remains unchanged.

## Out of Scope

1. Do not implement builder control for unrequested expensive sections in v0.6.0.
2. Do not implement `-Intent=`.
3. Do not implement profile modes such as `summary_only`, `digest_only`, or `ai_context`.
4. Do not implement natural language query handling.
5. Do not implement asset index, section index, lazy section dump, or query mode.
6. Do not rename existing JSON fields in full dump mode.
7. Do not remove existing fields from full dump mode.
8. Do not change `AssetDump.Build.cs` unless compilation proves an additional dependency is required.
9. Do not modify project gameplay assets.

## Constraints

1. This must be an additive, backward-compatible change for existing default dump commands.
2. Invalid section names must fail clearly before producing misleading output.
3. Output ordering must remain deterministic.
4. New structs, fields, and helper functions must have Korean one-line summary comments.
5. File and class names must remain under 32 characters where applicable.
6. The first implementation may filter output only at serialization time; builder control belongs to v0.6.1.
7. Full dump mode must continue to satisfy existing Plugin validation.
8. Widget Designer validation must still pass in full mode.
9. Section names must match `SectionRegistry_v1.md` unless this TaskSource is explicitly revised.
10. If a requested section is not applicable to an asset type, omit the specialized section safely rather than crashing.

## Target Files

Initial target files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Implementation also required these supporting files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
```

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/WidgetDesignerPlan/WidgetDesignerPlan_v1.md
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpWidgetTree.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpWidgetTree.cpp
```

## Current Behavior

AssetDump currently supports asset-level and batch-level filtering, such as root path, class filter, changed-only behavior, dependency inclusion, and max asset count.

However, once an asset is selected for dumping, major JSON sections are generally emitted together. AI workflows can choose which files or assets to read after the dump, but cannot yet request a smaller section-gated output directly from the dump command.

Current AI-oriented specialized output exists for WidgetBlueprint Designer hierarchy through `widget_designer`, but it is not yet controlled by a general `-Sections=` option.

## Desired Behavior

AssetDump should support explicit section-gated output.

Example command intent:

```text
-Sections=summary,digest
```

Expected major sections:

```text
summary
digest
```

Expected excluded major sections:

```text
details
graphs
references
widget_designer
```

Example command intent:

```text
-Sections=summary,digest,widget_designer
```

Expected major sections for WidgetBlueprint assets:

```text
summary
digest
widget_designer
```

Expected behavior for non-WidgetBlueprint assets:

```text
summary and digest are emitted.
widget_designer is omitted safely or emitted only as an explicitly empty section if that is the existing serializer policy.
The policy must be deterministic and documented in code comments or validation notes.
```

When `-Sections=` is omitted, AssetDump should behave as it does before v0.6.0.

## Section Names

The initial valid section names are:

```text
summary
digest
details
graphs
references
widget_designer
```

Unknown names such as `foo` or `widget-tree` must fail clearly.

The error message should include the invalid name and the supported section list.

## Minimal Envelope Policy

Even when section filtering is active, output may keep minimal fields that identify the dump.

Suggested minimal fields:

```text
schema_version
asset_path
asset_name
asset_class
dump_status
```

Codex should inspect the existing JSON structure before deciding the exact envelope fields. The goal is to avoid breaking the identity of reduced dump files while still excluding unrequested major sections.

## Implementation Steps

1. Inspect the existing commandlet argument parsing flow in `AssetDumpCommandlet.cpp`.
2. Inspect existing dump option or command configuration structures in `ADumpTypes.h`.
3. Add a section selection representation such as a small struct, enum flags, or set of canonical names.
4. Add Korean one-line comments above any new structs, fields, and helper functions.
5. Parse `-Sections=` as a comma-separated list.
6. Trim whitespace and normalize section names to lowercase.
7. Reject unknown section names with a clear error.
8. Treat missing `-Sections=` as full mode.
9. Propagate selected section options to JSON serialization.
10. In `ADumpJson.cpp`, gate major section emission based on section selection.
11. Ensure `summary` and `digest` can be independently controlled if the existing JSON structure allows it.
12. Ensure `widget_designer` emission is controlled by full mode or explicit `widget_designer` selection.
13. Preserve output ordering for all emitted sections.
14. Add or update validation/smoke test logic in `AssetDumpCommandlet.cpp` if an existing validation framework can cover section output.
15. If validation framework cannot easily test command-line section output, document the manual smoke test commands in code comments or validation notes and keep Plugin validation passing.
16. Build the project.
17. Run Plugin validation.
18. Run section smoke tests for summary/digest and widget_designer selections.

## Acceptance Criteria

1. Running AssetDump without `-Sections=` produces full dump output compatible with the current default behavior.
2. `-Sections=summary,digest` emits `summary` and `digest` among major sections and excludes `details`, `graphs`, `references`, and `widget_designer`.
3. `-Sections=summary,digest,widget_designer` emits Widget Designer output for WidgetBlueprint assets.
4. Unknown section names fail with a clear message that includes the invalid section name.
5. The valid section name list includes `summary`, `digest`, `details`, `graphs`, `references`, and `widget_designer`.
6. Existing Plugin validation passes in default full mode.
7. Existing Widget Designer validation passes in default full mode.
8. Reduced section output remains valid JSON.
9. Reduced section output still contains enough envelope data to identify the asset and dump status.
10. Existing full-mode JSON consumers are not required to change.

## Verification

Run build:

```text
CarFight_ReEditor Win64 Development
```

Run plugin validation:

```text
RunBPDumpRegression.ps1 -ValidationProfile Plugin -SkipBuild -CompactLog
```

Run section smoke test for compact overview:

```text
Dump a known WidgetBlueprint with -Sections=summary,digest.
Confirm summary and digest exist.
Confirm details, graphs, references, and widget_designer are absent among major sections.
```

Run section smoke test for Widget Designer:

```text
Dump /Game/CarFight/UI/WBP_VehicleDebugPanel or another known WidgetBlueprint with -Sections=summary,digest,widget_designer.
Confirm widget_designer.schema_version is widget_designer_v1.
Confirm widget_designer.node_count is greater than 0 when RootWidget exists.
```

Run invalid section smoke test:

```text
Run with -Sections=summary,invalid_section.
Confirm the command fails clearly and lists valid section names.
```

## Implementation Result

Status: completed.

Completion time reported:

```text
2026-07-10 11:14:14 +09:00
```

Actual verification result:

```text
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture: 8/8 succeeded
Plugin validation: 8/8 succeeded
Required failures: 0
Project batch: 43/43 succeeded
Changed-only: 43/43 skipped correctly
git diff --check: passed
```

Actual section smoke tests:

```text
-Sections=summary,digest
  exit_code: 0
  emitted_sections: summary,digest
  dump_status: success

-Sections=summary,digest,widget_designer
  exit_code: 0
  widget_designer.schema_version: widget_designer_v1
  widget_designer.node_count: 8

-Sections=summary,invalid_section
  exit_code: 1
  invalid name and all 6 valid sections were reported clearly
```

Validation artifact:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

## Migration

Existing commands do not need to change.

When `-Sections=` is omitted, AssetDump remains full-output compatible.

AI/MCP workflows may opt into reduced output by passing `-Sections=` once v0.6.0 is implemented.

Because section selection now participates in the changed-only fingerprint, existing changed-only outputs may regenerate once after the update. Subsequent runs should skip normally.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_0_Sections_CodexTask.yaml`

## Changelog

### v1.1

- Marked v0.6.0 implementation as completed.
- Added implementation result, actual verification result, actual changed files, and fingerprint migration note.
- Documented additional files required beyond the initial target list.

### v1.0

- Created Codex-compilable TaskSource for AssetDump v0.6.0 `-Sections=` option.
- Added section selection scope, constraints, acceptance criteria, and verification gates.
