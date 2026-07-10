# AssetDump Section Registry

## Metadata

- document_version: v1.3
- created_at: 2026-07-10
- updated_at: 2026-07-10
- owner_project: CarFight
- target_plugin: AssetDump
- document_role: shared_registry
- codex_input: false
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`

## Purpose

This registry defines canonical AssetDump section names, intent-to-section mappings, and section schema policy for future AI-oriented AssetDump work.

TaskSource documents should reference this registry rather than redefining section names independently.

## Section Naming Rules

1. Section names must be lowercase snake_case.
2. Section names must be stable once released.
3. New sections must declare a schema version if they expose structured data.
4. Section output should be deterministic.
5. Specialized sections should provide a compact preview when possible.
6. Section names used in commandlet options must match this registry unless a TaskSource explicitly updates this registry.

## Core Sections

| Section | Purpose | Existing or Planned | Schema Policy |
|---|---|---:|---|
| `summary` | Compact structured metadata and counts | Existing | Part of main dump schema |
| `digest` | AI/human-friendly compressed overview | Existing | Part of main dump schema |
| `details` | Detailed property/object information | Existing | Main dump schema or details schema |
| `graphs` | Blueprint graph data | Existing | Graph schema |
| `references` | Asset references/dependencies | Existing | Reference schema |

## Specialized Sections

| Section | Purpose | Existing or Planned | Schema Version |
|---|---|---:|---|
| `widget_designer` | WidgetBlueprint Designer hierarchy | Existing | `widget_designer_v1` |
| `data_asset_values` | DataAsset field/value summary | Planned v0.7.0 | `data_asset_values_v1` |
| `data_asset_diff` | DataAsset before/after field diff | Planned v0.7.1 | `data_asset_diff_v1` |
| `input_summary` | Enhanced Input Action/Mapping summary | Planned v0.7.2 | `input_summary_v1` |
| `component_tree` | Actor/Blueprint component hierarchy | Planned v0.7.3 | `component_tree_v1` |
| `material_param_summary` | Material and MaterialInstance parameter summary | Draft v0.7.4 | `material_param_summary_v1` |
| `blueprint_graph_digest` | AI-oriented Blueprint logic summary | Planned v0.8.0 | `bp_graph_digest_v1` |
| `bp_search_index` | Blueprint symbol/function/variable search index | Planned v0.8.2 | `bp_search_index_v1` |
| `asset_index` | Project-wide asset index | Planned v0.9.0 | `asset_index_v1` |
| `section_index` | Section and symbol lookup index | Planned v0.9.1 | `section_index_v1` |
| `query_result` | Structured query output | Planned v1.0.1 | `query_result_v1` |
| `ai_context_bundle` | Compact context package for AI | Planned v1.0.2 | `ai_context_bundle_v1` |

## Current Specialized Section Task

The current Codex task is v0.7.0 DataAsset Values.

```text
section: data_asset_values
schema_version: data_asset_values_v1
implementation_status: planned / not yet implemented
source_task: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_7_0_DataAssetValues_TaskSource.md
generated_contract: UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_7_0_DataAssetValues_CodexTask.yaml
```

The reserved `data_asset_values` Intent must remain unavailable until the section implementation and validation have completed.

## Initial Section Set for v0.6.0

The first `-Sections=` implementation should support at least:

```text
summary
digest
details
graphs
references
widget_designer
```

`widget_designer` should remain available for WidgetBlueprint assets when requested and should be omitted when not requested, unless full dump mode is active.

## Section Selection Semantics

### Full Mode

When no `-Sections=` option is provided, AssetDump should behave as full dump mode.

Expected behavior:

```text
- Preserve existing output compatibility.
- Emit the same major sections that current consumers expect.
- Do not require users to update existing commands.
```

### Explicit Section Mode

When `-Sections=` is provided, AssetDump should emit only the requested major sections plus any required minimal envelope fields.

Example:

```text
-Sections=summary,digest
```

Expected major sections:

```text
summary
digest
```

Excluded major sections:

```text
details
graphs
references
widget_designer
```

### Required Envelope Fields

Even in explicit section mode, AssetDump may keep minimal envelope fields required to identify the asset and validate the dump.

Suggested envelope fields:

```text
schema_version
asset_path
asset_name
asset_class
dump_status
```

The exact envelope policy should be defined by the relevant TaskSource and implementation.

## Invalid Section Policy

Unknown section names should produce a clear failure.

Example:

```text
-Sections=summary,unknown_section
```

Expected outcome:

```text
Dump command fails before writing misleading output.
Error message identifies unknown_section and lists valid section names.
```

## Intent Registry

AssetDump v0.6.2 introduces the initial intent set using only sections already implemented in v0.6.x.

### Implemented Initial Intents

| Intent | Sections |
|---|---|
| `quick_overview` | `summary,digest` |
| `widget_layout` | `summary,digest,widget_designer` |
| `blueprint_logic` | `summary,digest,graphs,references` |
| `dependency_trace` | `summary,digest,references` |

### Reserved Future Intents

These intent names remain planned but must not be accepted until their required specialized sections exist.

| Intent | Required Sections |
|---|---|
| `data_asset_values` | `summary,digest,data_asset_values,references` |
| `input_bindings` | `summary,digest,input_summary,references` |
| `component_overview` | `summary,digest,component_tree,references` |

### Intent Precedence

```text
-Sections only -> explicit sections control output and builders
-Intent only -> mapped sections control output and builders
-Intent plus -Sections -> explicit sections take precedence
neither option -> full mode
```

## Profile Registry

AssetDump v0.6.3 defines the initial profile contract as named selection presets over the existing section and intent systems.

| Profile | Effective Selection |
|---|---|
| `full` | Existing full dump behavior |
| `summary_only` | `summary` |
| `digest_only` | `summary,digest` |
| `ai_context` | `summary,digest` unless an explicit Intent has higher priority |

`ai_context` is a compact selection profile. It is not the future `ai_context_bundle_v1` export planned for v1.0.2.

### Global Selection Precedence

```text
Sections > Intent > Profile > implicit full
```

Expected examples:

```text
-Profile=summary_only
  section_source: profile
  sections: summary

-Profile=ai_context -Intent=widget_layout
  section_source: intent
  sections: summary,digest,widget_designer

-Profile=ai_context -Intent=widget_layout -Sections=summary,digest
  section_source: sections
  sections: summary,digest
```

Requested lower-priority profile or intent metadata may remain visible in the request envelope, but only the highest-priority source controls output and builders.

## Compatibility Policy

1. Default full mode must remain backward compatible.
2. Explicit section mode may omit fields that old consumers expect, because the user explicitly requested a reduced output.
3. New specialized sections must not rename or remove existing fields.
4. Deprecated section names must remain aliased for at least one minor version if introduced later.

## Validation Policy

Each new section should provide validation gates appropriate to the section.

Recommended validation checks:

```text
section exists when requested
section is absent when excluded
schema_version matches expected value
preview count is within budget
count fields match actual arrays where applicable
full mode remains compatible
```

## Migration

No runtime migration is required for this registry. Implementation TaskSource documents should update this registry when adding or renaming section names.

## Unresolved

None.

## Changelog

### v1.3

- Added v0.7.0 DataAsset Values as the current specialized-section task.
- Recorded `data_asset_values_v1` as planned but not yet implemented.
- Kept the reserved `data_asset_values` Intent disabled until implementation and validation complete.

### v1.2

- Defined the initial v0.6.3 profile set.
- Added the global `Sections > Intent > Profile > implicit full` precedence policy.
- Clarified that `ai_context` is a selection profile rather than `ai_context_bundle_v1`.

### v1.1

- Defined the initial v0.6.2 implemented intent set.
- Separated future reserved intents from currently supported intents.
- Added explicit Sections precedence policy.

### v1.0

- Created canonical section registry for AssetDump Asset Intelligence work.
- Defined initial v0.6.0 section set.
- Defined planned intent and profile names for later TaskSource documents.
