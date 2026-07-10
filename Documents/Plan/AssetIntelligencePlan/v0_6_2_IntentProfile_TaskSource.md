# AssetDump v0.6.2 Intent Profile Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.6.2
- implementation_status: completed
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_2_IntentProfile_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_2_IntentProfile_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- section_registry: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`

## Goal

Add an `-Intent=` option that maps common AI analysis goals to the existing AssetDump section selection and builder control system.

The implementation must reuse the canonical section selection introduced in v0.6.0 and the builder execution planning introduced in v0.6.1. Intent handling must resolve to canonical sections before serialization, builder planning, validation, and changed-only fingerprinting.

## In Scope

1. Add commandlet argument parsing for a single `-Intent=` value.
2. Define canonical intent names and intent-to-section mappings.
3. Support these initial intents only:
   - `quick_overview`
   - `widget_layout`
   - `blueprint_logic`
   - `dependency_trace`
4. Resolve each intent to existing v0.6.x sections:
   - `quick_overview` -> `summary,digest`
   - `widget_layout` -> `summary,digest,widget_designer`
   - `blueprint_logic` -> `summary,digest,graphs,references`
   - `dependency_trace` -> `summary,digest,references`
5. Reuse the existing section normalization, output filtering, and builder planning paths.
6. Define precedence when both `-Intent=` and `-Sections=` are provided.
7. Use explicit `-Sections=` as the effective section source when both options are provided.
8. Record intent resolution in the request envelope.
9. Preserve `request.builder_sections` from v0.6.1 as the actual builder execution plan.
10. Add intent information to changed-only fingerprint inputs when it affects dump output or request metadata.
11. Fail clearly for unknown intent names before output generation.
12. Add automatic validation or smoke checks for intent resolution and precedence.

## Out of Scope

1. Do not implement natural-language parsing.
2. Do not infer intent from asset class or user text.
3. Do not implement intents that require sections not yet available, including:
   - `data_asset_values`
   - `input_bindings`
   - `component_overview`
4. Do not implement profile modes such as `summary_only`, `digest_only`, or `ai_context`.
5. Do not implement asset index, section index, lazy section dump, or query mode.
6. Do not add new specialized asset sections in this task.
7. Do not change existing section names.
8. Do not change existing full-mode JSON schema except for additive request metadata if the current request object is already emitted.
9. Do not modify gameplay code or project assets.
10. Do not change `AssetDump.Build.cs` unless compilation proves an additional dependency is required.

## Constraints

1. Omitting both `-Intent=` and `-Sections=` must preserve existing full mode.
2. Omitting `-Intent=` while using `-Sections=` must preserve v0.6.0/v0.6.1 behavior.
3. Explicit `-Sections=` must take precedence over `-Intent=` when both are present.
4. Intent resolution must produce canonical lowercase section names in deterministic order.
5. Unknown intent names must fail before misleading output is written.
6. Error output must include the invalid intent and all valid intent names.
7. New structs, fields, enums, and helper functions must have Korean one-line summary comments.
8. File and class names must remain under 32 characters where applicable.
9. Existing Plugin validation and v0.6.x section/builder checks must continue to pass.
10. Builder control must continue to use conservative prerequisites for references.
11. Changed-only behavior must remain stable after any expected fingerprint refresh.
12. Intent mappings must use only sections registered and implemented at v0.6.2.

## Intent Registry for v0.6.2

```text
quick_overview
  sections: summary,digest
  purpose: fast asset overview with minimal cost

widget_layout
  sections: summary,digest,widget_designer
  purpose: inspect WidgetBlueprint Designer hierarchy

blueprint_logic
  sections: summary,digest,graphs,references
  purpose: inspect Blueprint logic and its references

dependency_trace
  sections: summary,digest,references
  purpose: inspect asset reference relationships
```

Future intent names remain reserved in planning documents but must not be accepted until their required sections exist.

## Precedence Policy

When neither option is provided:

```text
mode: full
section_source: full
```

When only `-Intent=` is provided:

```text
mode: explicit
section_source: intent
resolved_sections: mapping for the canonical intent
```

When only `-Sections=` is provided:

```text
mode: explicit
section_source: sections
resolved_sections: normalized explicit sections
```

When both are provided:

```text
mode: explicit
section_source: sections
resolved_sections: normalized explicit sections
intent: retain the canonical requested intent as request metadata if practical
```

Explicit sections must control output and builders in the combined case.

## Request Metadata

The request envelope should expose enough information to explain how output was selected.

Recommended additive fields for explicit intent mode:

```text
request.intent
request.section_source
request.sections
request.builder_sections
```

Expected meanings:

```text
intent
  Canonical requested intent, or empty/omitted when no intent was supplied.

section_source
  full, intent, or sections.

sections
  Effective canonical output section list.

builder_sections
  Actual builder execution plan from v0.6.1.
```

Use existing request field naming/style if equivalent fields already exist. Do not create duplicate concepts under different names.

## Target Files

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

## Current Behavior

AssetDump v0.6.1 supports explicit section selection and builder control.

Current behavior:

```text
- -Sections= controls emitted major sections.
- Explicit sections control the actual builder phase.
- request.builder_sections reports the actual builder plan.
- Full mode remains compatible with existing IncludeSummary/Details/Graphs/References behavior.
- Unknown section names fail before output generation.
- Changed-only fingerprint includes section selection.
```

AI or MCP callers must still know the exact section list for each analysis purpose.

## Desired Behavior

AI or MCP callers should be able to specify a stable analysis goal without manually maintaining a section list.

Examples:

```text
-Intent=quick_overview
  effective sections: summary,digest
  expected builder_sections: summary

-Intent=widget_layout
  effective sections: summary,digest,widget_designer
  expected builder_sections: summary,widget_designer

-Intent=blueprint_logic
  effective sections: summary,digest,graphs,references
  expected builder_sections: conservative plan required by graphs/references

-Intent=dependency_trace
  effective sections: summary,digest,references
  expected builder_sections: conservative references prerequisite plan
```

Combined example:

```text
-Intent=widget_layout -Sections=summary,digest
```

Expected result:

```text
section_source: sections
output sections: summary,digest
builder_sections: summary
widget_designer omitted
```

## Implementation Steps

1. Inspect the canonical section model in `ADumpTypes.*` and `ADumpRunOpts.*`.
2. Add a compact intent representation or canonical intent helper using existing style.
3. Add Korean one-line comments above new intent-related declarations and helpers.
4. Parse `-Intent=` as one normalized lowercase name.
5. Reject empty or unknown intent values clearly.
6. Map supported intents to canonical section selections.
7. Apply precedence so explicit `-Sections=` overrides intent-derived sections.
8. Resolve the effective section selection before builder planning.
9. Ensure `ADumpService.cpp` uses the effective sections and preserves v0.6.1 builder behavior.
10. Add request metadata for canonical intent and section source using existing JSON naming style.
11. Preserve `request.builder_sections` as actual execution evidence.
12. Update changed-only fingerprint inputs to include intent/request selection metadata where required for deterministic changed-only behavior.
13. Add validation cases for all four intents.
14. Add validation for combined `-Intent=` and `-Sections=` precedence.
15. Add validation for unknown intent failure.
16. Build the project.
17. Run Plugin validation.
18. Run project batch and changed-only verification.

## Acceptance Criteria

1. Omitting both `-Intent=` and `-Sections=` preserves full mode behavior.
2. `-Intent=quick_overview` resolves to `summary,digest`.
3. `-Intent=quick_overview` reports `builder_sections=summary`.
4. `-Intent=widget_layout` resolves to `summary,digest,widget_designer`.
5. `-Intent=widget_layout` emits a valid `widget_designer_v1` section for the Widget fixture.
6. `-Intent=blueprint_logic` resolves to `summary,digest,graphs,references`.
7. `-Intent=dependency_trace` resolves to `summary,digest,references`.
8. References-related intents retain the conservative prerequisite builder plan required for correct reference extraction.
9. When both Intent and Sections are provided, explicit Sections control output and builders.
10. Request metadata identifies the canonical intent and whether effective sections came from intent or explicit sections.
11. Unknown intent values fail before output generation and list all valid intents.
12. Existing invalid section behavior remains unchanged.
13. Existing Plugin validation and v0.6.x section/builder checks pass.
14. Project batch succeeds.
15. Changed-only rerun skips unchanged assets normally.
16. Existing commands require no migration.

## Verification

Run build:

```text
CarFight_ReEditor Win64 Development
```

Run Plugin validation:

```text
RunBPDumpRegression.ps1 -ValidationProfile Plugin -SkipBuild -CompactLog
```

Run intent smoke tests:

```text
-Intent=quick_overview
  confirm sections: summary,digest
  confirm builder_sections: summary

-Intent=widget_layout
  confirm sections: summary,digest,widget_designer
  confirm builder_sections: summary,widget_designer
  confirm widget_designer.schema_version: widget_designer_v1
  confirm node_count > 0

-Intent=blueprint_logic
  confirm sections: summary,digest,graphs,references
  confirm references succeeds

-Intent=dependency_trace
  confirm sections: summary,digest,references
  confirm references succeeds
```

Run precedence smoke test:

```text
-Intent=widget_layout -Sections=summary,digest
  confirm section_source: sections
  confirm output sections: summary,digest
  confirm builder_sections: summary
  confirm widget_designer is not emitted
```

Run invalid intent test:

```text
-Intent=invalid_intent
  confirm exit code is non-zero
  confirm invalid_intent is shown
  confirm all valid intent names are listed
```

Run project and changed-only checks:

```text
Run project batch.
Run changed-only batch again.
Confirm unchanged assets are skipped normally.
```

## Implementation Result

Status: completed.

Completion time reported:

```text
2026-07-10 13:29:10 +09:00
```

Actual changed files:

```text
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpTypes.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpTypes.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Public/ADumpRunOpts.h
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpRunOpts.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpService.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpJson.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/ADumpFingerprint.cpp
UE/Plugins/ue-assetdump/Source/AssetDump/Private/AssetDumpCommandlet.cpp
```

Verification result:

```text
AssetDump module compile and DLL link: succeeded
Plugin validation: 8/8 succeeded
Intent/section smoke checks: 12/12 succeeded
Project batch: 43/43 succeeded
Changed-only rerun: 43/43 skipped correctly
```

Full project build result:

```text
CarFight_ReEditor Win64 Development: failed
cause: 2 existing errors in CFVehiclePawn.cpp
classification: unrelated_existing_issue
```

The v0.6.2 feature is accepted at the AssetDump plugin scope. The gameplay build errors must be handled by a separate task and must not be fixed as part of AssetDump work.

Observed feature evidence:

```text
widget_layout
  widget_designer.schema_version: widget_designer_v1
  widget_designer.node_count: 8

blueprint_logic and dependency_trace
  references emitted: true

-Intent=widget_layout -Sections=summary,digest
  request.section_source: sections
  request.builder_sections: summary

invalid_intent
  exit_code: 1
  complete valid intent list reported
```

Validation artifact:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

## Migration

Existing commands do not need to change.

`-Intent=` is optional. Existing full-mode and `-Sections=` commands retain their behavior.

When both options are used, explicit `-Sections=` takes precedence. AI/MCP callers should normally use one selection mechanism per request unless intentionally overriding an intent mapping.

A changed-only refresh may occur once if intent metadata becomes part of fingerprint inputs. Subsequent unchanged runs must skip normally.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_2_IntentProfile_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_2_IntentProfile_CodexTask.yaml`

## Changelog

### v1.1

- Marked v0.6.2 implementation as completed.
- Added actual changed files, validation results, build classification, and migration notes.
- Documented the unrelated existing `CFVehiclePawn.cpp` build errors and plugin-scope acceptance.

### v1.0

- Promoted v0.6.2 Intent Profile from draft to Codex-compilable TaskSource.
- Limited initial intent mappings to sections already implemented in v0.6.x.
- Defined explicit Sections precedence, request metadata, fingerprint policy, acceptance criteria, and verification gates.
