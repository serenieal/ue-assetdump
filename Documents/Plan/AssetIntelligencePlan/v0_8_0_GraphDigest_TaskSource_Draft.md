# AssetDump v0.8.0 Graph Digest Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v0.8.0
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`

## Goal

Add an AI-oriented Blueprint graph digest that classifies common Blueprint node roles and summarizes logic without requiring AI to read full graph output.

## Draft Node Roles

```text
event
function_call
variable_get
variable_set
branch
loop
cast
spawn
delegate_bind
input_action
timeline
latent
```

## Draft Scope

1. Summarize events.
2. Summarize function calls.
3. Summarize variable reads and writes.
4. Provide schema version `bp_graph_digest_v1`.
5. Provide compact preview suitable for AI context.

## Draft Out of Scope

1. Do not implement full execution path preview in this task unless revised.
2. Do not replace existing graph output.
3. Do not implement query mode.

## Unresolved

This draft must be refined after current graph dump schema and node metadata are reviewed.

## Changelog

### v0.1-draft

- Added placeholder for v0.8.0 Blueprint Graph Digest planning.
