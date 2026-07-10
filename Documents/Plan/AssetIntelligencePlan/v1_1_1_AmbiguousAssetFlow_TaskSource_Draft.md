# AssetDump v1.1.1 Ambiguous Asset Candidate Flow Task Source Draft

## Metadata

- document_version: v0.1-draft
- created_at: 2026-07-10
- target_assetdump_version: v1.1.1
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source_draft
- codex_input: false
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v1_1_0_NaturalQueryAdapter_TaskSource_Draft.md`

## Goal

Define how AI tooling should handle ambiguous natural-language asset references by returning ranked candidate assets instead of guessing silently.

## Draft Scope

1. Define candidate result schema.
2. Define ranking hints such as exact name match, path relevance, class family, and section availability.
3. Define follow-up prompt policy for ambiguous cases.
4. Integrate with asset index and query adapter documentation.

## Draft Out of Scope

1. Do not implement in-plugin natural language parsing.
2. Do not require a single candidate when confidence is low.
3. Do not replace structured query mode.

## Unresolved

This draft must be refined after Natural Query Adapter exists.

## Changelog

### v0.1-draft

- Added placeholder for v1.1.1 Ambiguous Asset Candidate Flow planning.
