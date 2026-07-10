# AssetDump v0.6.4 Section Smoke Tests Task Source Draft

## Metadata

- document_version: v0.2-draft
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.6.4
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: superseded_draft
- codex_input: false
- superseded_by: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md`
- replacement_reason: section, intent, and profile smoke coverage already reached 19/19 in v0.6.3

## Status

This draft is obsolete because the planned section smoke coverage was implemented incrementally in v0.6.0 through v0.6.3.

Current validation evidence:

```text
section/intent/profile check_count: 19
failure_count: 0
```

The remaining v0.6.4 work is regression harness hardening:

```text
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_4_RegressionHarness_TaskSource.md
```

That task addresses:

```text
- stale hard-coded EngineRoot default
- deterministic local engine path resolution
- strict process-exit versus fresh-report classification
- known external UE HttpListener port conflict
- negative safety tests preventing false success
```

## Unresolved

Superseded by the finalized regression harness TaskSource.

## Changelog

### v0.2-draft

- Marked the section-smoke draft as superseded because 19 automatic checks already pass.
- Redirected v0.6.4 to Regression Harness Hardening.

### v0.1-draft

- Added placeholder for v0.6.4 Section Smoke Tests planning.
