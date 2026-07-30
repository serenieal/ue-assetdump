# AssetDump v1.1.0 Natural Query Adapter Decision Record

## Metadata

- document_version: v2.1
- created_at: 2026-07-30
- updated_at: 2026-07-30
- task_id: `ADUMP-v1.1.0-NQAC`
- status: `Cancelled / Superseded by MCP Direct Query Orchestration / Contract Not Accepted`
- owner_repository: `assetdump_repo`
- document_role: historical_architecture_decision
- accepted_runtime_baseline: `ADUMP-v1.0.2-AICB`

## Decision

AssetDump does not expose a natural-language request bridge for normal MCP usage.

```text
User request
  -> AI interprets intent
  -> MCP constructs accepted Query Mode options
  -> AssetDump performs indexed bounded retrieval
  -> query_result_v1 or ai_context_bundle_v1
  -> AI explains the result
```

The proposed `queryadapter` command mode, `natural_query_request_v1`, `assetdump_query_request_v1`, request-text/language normalization and `ADUMP_NQ_*` stable-code family were retired before contract acceptance.

## Rationale

The project objective is to prevent a complete Unreal resource dump from becoming the minimum unit of work and to keep AI context bounded. It is not to make AssetDump understand natural language.

That scale-control objective is already implemented by accepted contracts:

```text
v0.9.0 Asset Index
v0.9.1 Section Index
v0.9.2 Lazy Section Dump
v0.9.3 bounded Dependency Trace Query
v1.0.0 Query Mode
v1.0.1 Query Result Schema
v1.0.2 bounded AI Context Bundle
```

AI/MCP already interprets the user's language and intent. Passing the original text and language metadata into AssetDump would duplicate caller responsibility without reducing retrieval scope.

## Ownership Boundary

### AI/MCP owns

- natural-language interpretation;
- ambiguity resolution and user follow-up;
- exact `asset_id` or `object_path` selection;
- `section | dependency` route selection;
- explicit section lists and dependency bounds;
- one or more accepted Query Mode calls;
- result explanation.

### AssetDump owns

- structured command-option validation;
- indexed asset and section lookup;
- bounded dependency traversal;
- deterministic `query_result_v1` output;
- bounded `ai_context_bundle_v1` export;
- stable failures for accepted contracts.

## Historical Evidence

The cancelled implementation reached Level 1 and BuildPlugin PASS, then produced a failed canonical Phase 2 diagnostic report. Those artifacts remain implementation history only. They do not establish a public contract and are not repair, retry, Phase 1 or acceptance prerequisites.

The two temporary Source files were untracked v0.2.0 compile-neutral retired stubs with no includes, symbols, declarations or runtime implementation. A later Codex deletion-only task removed both files, confirmed no commandlet, Source-tree or Phase 2 runner references, and preserved identical SHA-256 values for all 11 protected dirty files.

## Migration

MCP callers construct accepted `-Mode=query` options directly. Existing Query Mode, Query Result and AI Context Bundle callers require no migration. The deleted adapter Source paths must not be recreated as part of maintenance or future MCP orchestration work.

A future Unreal Editor Window that accepts natural language requires a separate explicit Plan and must not reactivate this cancelled contract implicitly.

## Changelog

### v2.1 - 2026-07-30

- Recorded final deletion of both untracked compile-neutral adapter Source files by the Codex deletion-only task.
- Confirmed no NQAC runtime reference or retired request-contract string remains in AssetDump Source or the Phase 2 runner.
- Recorded preservation of all 11 protected dirty-file SHA-256 values and no build, phase, commit or push action.
- Superseded the temporary retained-stub explanation without changing the cancellation decision or accepted v1.0.2 baseline.

### v2.0 - 2026-07-30

- Cancelled NQAC before acceptance.
- Moved natural-language interpretation and orchestration ownership to AI/MCP.
- Restored the accepted v1.0.2 bounded-query baseline as the current architecture.
- Reclassified the implementation and failed runtime report as historical evidence only.
