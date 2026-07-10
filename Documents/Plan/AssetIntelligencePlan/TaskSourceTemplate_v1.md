# AssetDump Codex TaskSource Template

## Metadata

- document_version: v1.0
- created_at: 2026-07-10
- document_role: template
- codex_input: false
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`

## Purpose

Use this template when creating future Codex-compilable AssetDump TaskSource documents.

A TaskSource is not just a roadmap note. It is a contract that `plan.inspect_unresolved` and `plan.compile_outputs` should be able to process into a Codex task.

## Required Sections

Copy this structure for future implementation tasks:

```md
# AssetDump vX.Y.Z Feature Name Task Source

## Metadata

- document_version: v1.0
- created_at: YYYY-MM-DD
- target_assetdump_version: vX.Y.Z
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/<TaskSourceFile>.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/<CodexTaskFile>.yaml`

## Goal

State the implementation goal in one or two direct paragraphs.

## In Scope

1. List what Codex must implement.

## Out of Scope

1. List what Codex must not implement.

## Constraints

1. List compatibility, naming, safety, and coding constraints.

## Target Files

```text
UE/Plugins/ue-assetdump/Source/AssetDump/...
```

## Reference Files

```text
UE/Plugins/ue-assetdump/Documents/Plan/...
```

## Current Behavior

Describe the current behavior that must be changed or extended.

## Desired Behavior

Describe the target behavior with examples.

## Implementation Steps

1. Provide ordered, concrete steps.

## Acceptance Criteria

1. Provide observable criteria.

## Verification

List build, validation, and smoke test commands or checks.

## Migration

Describe compatibility and user migration.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `...`
- output_target: `...`

## Changelog

### v1.0

- Initial TaskSource.
```

## Plan Tool Flow

Recommended workflow:

```text
1. Write TaskSource.
2. Run plan.inspect_unresolved.
3. Resolve blocking unresolved items.
4. Run plan.compile_outputs.
5. Review generated Codex YAML.
6. Hand generated YAML to Codex.
7. Record implementation and verification result back into the planning folder.
```

## Changelog

### v1.0

- Created reusable TaskSource template for AssetDump planning.
