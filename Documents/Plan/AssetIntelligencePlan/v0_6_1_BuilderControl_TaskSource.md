# AssetDump v0.6.1 Builder Control Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.6.1
- implementation_status: completed
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- section_registry: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

## Goal

Use the section selection option added in AssetDump v0.6.0 to avoid running expensive data builders for output sections that were not requested.

AssetDump v0.6.0 filters output at serialization time. AssetDump v0.6.1 should preserve that behavior while adding safe builder-level control so reduced section dumps avoid unnecessary graph, details, reference, and specialized section construction where possible.

## In Scope

1. Inspect the v0.6.0 section selection structures and helpers.
2. Reuse the existing section selection model instead of creating a competing option system.
3. Identify builder or collection paths for major sections:
   - `details`
   - `graphs`
   - `references`
   - `widget_designer`
4. Skip expensive builder work when the corresponding section is not requested and full mode is not active.
5. Preserve full mode behavior when `-Sections=` is omitted.
6. Preserve explicit section mode behavior introduced by v0.6.0.
7. Keep `summary` and `digest` safe and available when requested.
8. Ensure skipped sections do not leave misleading populated data in reduced outputs.
9. Keep changed-only fingerprint behavior from v0.6.0 intact.
10. Add or update smoke tests if the existing validation framework can verify builder control without fragile timing assumptions.
11. Add comments or implementation notes that distinguish output filtering from builder control.

## Out of Scope

1. Do not implement `-Intent=`.
2. Do not implement profile modes such as `summary_only`, `digest_only`, or `ai_context`.
3. Do not implement lazy section dump.
4. Do not implement asset index, section index, query mode, or natural language query handling.
5. Do not change section names.
6. Do not remove v0.6.0 serialization filtering.
7. Do not change existing full-mode JSON schema.
8. Do not optimize every small helper if it is not section-expensive.
9. Do not modify project gameplay assets.
10. Do not change `AssetDump.Build.cs` unless compilation proves an additional dependency is required.

## Constraints

1. This must be backward compatible in full mode.
2. Reduced section output must remain compatible with v0.6.0 behavior.
3. Builder control must be conservative. If skipping a builder risks incorrect summary or digest output, keep that builder enabled and document why.
4. New structs, fields, and helper functions must have Korean one-line summary comments.
5. File and class names must remain under 32 characters where applicable.
6. Output ordering must remain deterministic.
7. Existing Plugin validation must continue to pass.
8. Existing v0.6.0 section smoke checks must continue to pass.
9. Unknown section failure behavior from v0.6.0 must remain unchanged.
10. Changed-only skip behavior must remain stable after the first expected fingerprint refresh.

## Target Files

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
```

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/WidgetDesignerPlan/WidgetDesignerPlan_v1.md
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpWidgetTree.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpWidgetTree.cpp
```

## Current Behavior

AssetDump v0.6.0 supports `-Sections=` and can emit reduced JSON output.

Current behavior after v0.6.0:

```text
- Full mode remains default when -Sections is omitted.
- Explicit section mode serializes only requested major sections.
- widget_designer is emitted only in full mode or when explicitly requested.
- Unknown section names fail before output generation.
- Section selection participates in changed-only fingerprinting.
```

However, v0.6.0 is allowed to filter at JSON serialization time. That means some expensive section data may still be collected before it is omitted from output.

## Desired Behavior

AssetDump v0.6.1 should avoid building expensive section data when it cannot appear in output.

Examples:

```text
-Sections=summary,digest
  should avoid unnecessary details, graphs, references, and widget_designer builders when safe.

-Sections=summary,digest,widget_designer
  should allow widget_designer building for WidgetBlueprint assets but avoid unrelated expensive sections when safe.

-Sections omitted
  should keep current full dump behavior.
```

If a builder contributes data required by `summary` or `digest`, Codex must not skip it blindly. The correct behavior is to inspect the dependency and either:

```text
- keep the builder enabled for required summary/digest data, or
- split the minimal summary/digest collection from the expensive full section builder, if safe and localized.
```

## Implementation Steps

1. Inspect v0.6.0 section selection API in `ADumpTypes.*` and `ADumpRunOpts.*`.
2. Inspect builder call paths in `AssetDumpCommandlet.cpp` and `ADumpSummaryExt.cpp`.
3. Inspect JSON gating in `ADumpJson.cpp` to understand which section names control output.
4. Identify where details data is built and whether it is safe to skip in reduced modes.
5. Identify where graph data is built and whether it is safe to skip in reduced modes.
6. Identify where reference data is built and whether it is safe to skip in reduced modes.
7. Identify where `BuildWidgetDesignerSummary` or equivalent Widget Designer traversal is called.
8. Add conservative section-enabled checks around expensive builders where safe.
9. Preserve required summary/digest fields even when details, graphs, references, or widget_designer sections are not requested.
10. If a builder cannot be skipped safely, add a concise comment explaining why.
11. Ensure full mode still executes all builders as before.
12. Ensure invalid section parsing behavior is unchanged.
13. Ensure changed-only fingerprint still includes section selection.
14. Build the project.
15. Run Plugin validation.
16. Run section smoke tests for summary/digest and widget_designer selection.
17. Run changed-only validation to confirm stable skip behavior.

## Acceptance Criteria

1. Full mode with `-Sections=` omitted remains compatible with v0.6.0 full output.
2. `-Sections=summary,digest` still emits valid reduced JSON with summary and digest.
3. `-Sections=summary,digest` avoids at least one expensive unrequested section builder where safe.
4. `-Sections=summary,digest,widget_designer` still emits `widget_designer` for WidgetBlueprint assets.
5. `widget_designer` traversal is not run when `widget_designer` is not requested and full mode is not active, unless Codex proves it is required for requested summary or digest data.
6. Unknown section names still fail before output generation.
7. Existing Plugin validation passes.
8. Existing v0.6.0 section smoke checks pass.
9. Changed-only batch behavior remains stable.
10. Any builder that remains intentionally unskipped has a clear code comment or implementation note explaining the dependency.

## Verification

Run build:

```text
CarFight_ReEditor Win64 Development
```

Run plugin validation:

```text
RunBPDumpRegression.ps1 -ValidationProfile Plugin -SkipBuild -CompactLog
```

Run compact section smoke test:

```text
Dump a known WidgetBlueprint with -Sections=summary,digest.
Confirm summary and digest exist.
Confirm details, graphs, references, and widget_designer are absent among major sections.
```

Run Widget Designer section smoke test:

```text
Dump a known WidgetBlueprint with -Sections=summary,digest,widget_designer.
Confirm widget_designer.schema_version is widget_designer_v1.
Confirm widget_designer.node_count is greater than 0 when RootWidget exists.
```

Run invalid section smoke test:

```text
Run with -Sections=summary,invalid_section.
Confirm the command fails clearly and lists valid section names.
```

Run changed-only check:

```text
Run changed-only batch after a successful full batch.
Confirm unchanged assets are skipped normally.
```

Optional implementation evidence:

```text
Add logs, counters, or validation-only assertions only if existing validation style supports them without noisy output.
Do not add permanent noisy runtime logs just to prove a builder was skipped.
```

## Implementation Result

Status: completed.

Completion time reported:

```text
2026-07-10 12:52:43 +09:00
```

Actual changed files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpSummaryExt.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Actual verification result:

```text
CarFight_ReEditor Win64 Development: succeeded
AssetDump compile and DLL link: succeeded
New compile warnings: none
Plugin fixture: 8/8 succeeded
Plugin validation: 8/8 succeeded
Required failures: 0
Builder/section automatic checks: 6/6 succeeded
Project batch: 43/43 succeeded
Changed-only rerun: 43/43 skipped correctly
```

Builder evidence:

```text
-Sections=summary,digest
  request.builder_sections: summary
  details_ms: 0
  graphs_ms: 0
  references_ms: 0
  widget_designer_node_count: 0

-Sections=summary,digest,widget_designer
  request.builder_sections: summary,widget_designer
  widget_designer.schema_version: widget_designer_v1
  widget_designer.node_count: 8
```

The references path intentionally keeps conservative prerequisite summary/details/graphs inputs because reference extraction depends on them.

Validation artifact:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

## Migration

Existing commands do not need to change.

`-Sections=` behavior from v0.6.0 remains the user-facing interface. v0.6.1 is an internal performance and work-avoidance improvement.

If builder control changes fingerprint behavior, document the reason. Prefer preserving the v0.6.0 fingerprint semantics unless a change is necessary.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_1_BuilderControl_CodexTask.yaml`

## Changelog

### v1.1

- Marked v0.6.1 implementation as completed.
- Added actual changed files, validation results, builder execution evidence, and migration notes.
- Documented conservative reference builder prerequisites and `request.builder_sections`.

### v1.0

- Promoted v0.6.1 Builder Control from draft to Codex-compilable TaskSource.
- Added target files, constraints, acceptance criteria, and verification gates.
- Scoped the task as conservative builder-level control after v0.6.0 section serialization filtering.
