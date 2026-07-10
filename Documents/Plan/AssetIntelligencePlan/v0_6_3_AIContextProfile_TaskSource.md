# AssetDump v0.6.3 AI Context Profile Task Source

## Metadata

- document_version: v1.1
- created_at: 2026-07-10
- updated_at: 2026-07-10
- target_assetdump_version: v0.6.3
- implementation_status: completed
- owner_project: CarFight
- target_plugin: AssetDump
- artifact_role: codex_task_source
- codex_input: true
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_0_Sections_TaskSource.md`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_1_BuilderControl_TaskSource.md`
- depends_on: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_2_IntentProfile_TaskSource.md`
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_3_AIContextProfile_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_3_AIContextProfile_CodexTask.yaml`
- roadmap: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/AssetIntelligenceRoadmap_v1.md`
- section_registry: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md`
- validation_policy: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md`

## Goal

Add a `-Profile=` option that provides stable named presets for common AssetDump output modes while reusing the existing section selection, intent resolution, builder control, serialization, and changed-only fingerprint systems.

AssetDump v0.6.3 must not create a competing extraction path. A profile is a selection preset or full-mode preset that resolves before builder planning.

## In Scope

1. Add commandlet argument parsing for a single `-Profile=` value.
2. Support these canonical profile names:
   - `full`
   - `summary_only`
   - `digest_only`
   - `ai_context`
3. Resolve profiles as follows:
   - `full` -> existing full dump behavior
   - `summary_only` -> `summary`
   - `digest_only` -> `summary,digest`
   - `ai_context` without Intent -> `summary,digest`
4. Allow `ai_context` to compose with an explicit v0.6.2 Intent.
5. Apply one deterministic selection precedence policy across Sections, Intent, Profile, and implicit full mode.
6. Reuse v0.6.0 canonical section normalization and output filtering.
7. Reuse v0.6.1 builder planning and `request.builder_sections`.
8. Reuse v0.6.2 intent mapping and request metadata.
9. Add `request.profile` to the request envelope.
10. Extend `request.section_source` to report `profile` when a profile directly determines the effective output.
11. Include profile metadata and effective selection source in changed-only fingerprint inputs when required for deterministic output.
12. Fail clearly for unknown profile names before output generation.
13. Add automatic smoke checks for profile mappings, precedence, invalid profiles, and full-mode compatibility.

## Out of Scope

1. Do not implement a new AI context bundle schema.
2. Do not create a new top-level `ai_context` JSON section.
3. Do not implement token counting, token budgets, byte budgets, or truncation policies.
4. Do not implement natural-language query parsing.
5. Do not implement asset index, section index, lazy section loading, or query mode.
6. Do not add specialized asset sections.
7. Do not infer an Intent from asset class or content.
8. Do not change the existing four Intent mappings.
9. Do not rename existing sections, intents, or request fields.
10. Do not modify gameplay code or project assets.
11. Do not modify `CFVehiclePawn.cpp` to clear the unrelated current project build failure.
12. Do not change `AssetDump.Build.cs` unless compilation proves a new dependency is required.

## Constraints

1. Omitting `-Profile=`, `-Intent=`, and `-Sections=` must preserve existing implicit full mode.
2. Existing `-Sections=` commands must retain v0.6.0/v0.6.1 behavior.
3. Existing `-Intent=` commands must retain v0.6.2 behavior.
4. Selection precedence must be:
   - explicit Sections
   - explicit Intent
   - explicit Profile
   - implicit full mode
5. Canonical profile names must be lowercase snake_case.
6. Input profile names should follow the current normalization style used by Sections and Intent where practical.
7. Unknown profile names must fail before misleading output is written.
8. Unknown-profile errors must include the invalid profile and the full valid profile list.
9. Profile resolution must occur before builder planning.
10. Output and builder ordering must remain deterministic.
11. New structs, fields, enums, and helper functions must have Korean one-line summary comments.
12. File and class names must remain under 32 characters where applicable.
13. Plugin validation and all v0.6.x section, builder, and intent checks must continue to pass.
14. References-related intent behavior must preserve conservative prerequisite builders.
15. Changed-only behavior must become stable after any expected one-time fingerprint refresh.
16. The AssetDump module must compile and link.
17. A full editor build failure caused only by the existing two `CFVehiclePawn.cpp` errors must be reported as `unrelated_existing_issue`, not fixed in this task.

## Profile Registry for v0.6.3

```text
full
  behavior: existing full dump
  explicit_sections: none
  builder_behavior: existing full builder behavior

summary_only
  sections: summary
  expected_builder_sections: summary

digest_only
  sections: summary,digest
  expected_builder_sections: summary

ai_context
  default_sections_without_intent: summary,digest
  default_expected_builder_sections: summary
  with_explicit_intent: use the existing intent mapping
```

`digest_only` retains `summary` because the current digest contract is paired with summary and existing v0.6.x behavior already treats `summary,digest` as the compact overview set.

`ai_context` is a profile preset, not the final `ai_context_bundle_v1` export planned for v1.0.2.

## Selection Precedence Policy

### No Selection Options

```text
mode: full
section_source: full
profile: empty or omitted
intent: empty or omitted
```

### Profile Only

```text
-Profile=full
  mode: full
  section_source: profile

-Profile=summary_only
  effective sections: summary
  section_source: profile

-Profile=digest_only
  effective sections: summary,digest
  section_source: profile

-Profile=ai_context
  effective sections: summary,digest
  section_source: profile
```

### Intent and Profile

Explicit Intent takes precedence over Profile.

Example:

```text
-Profile=summary_only -Intent=widget_layout
```

Expected:

```text
profile: summary_only
intent: widget_layout
section_source: intent
sections: summary,digest,widget_designer
builder_sections: summary,widget_designer
```

Example:

```text
-Profile=ai_context -Intent=dependency_trace
```

Expected:

```text
profile: ai_context
intent: dependency_trace
section_source: intent
sections: summary,digest,references
builder_sections: conservative references plan
```

### Sections with Intent or Profile

Explicit Sections take precedence over both Intent and Profile.

Example:

```text
-Profile=ai_context -Intent=widget_layout -Sections=summary,digest
```

Expected:

```text
profile: ai_context
intent: widget_layout
section_source: sections
sections: summary,digest
builder_sections: summary
widget_designer emitted: false
```

Requested lower-priority metadata may remain in the request envelope, but it must not control output or builders.

## Request Metadata

The request envelope should expose the selection request and effective source without duplicating existing concepts.

Required additive field:

```text
request.profile
```

Existing fields to preserve:

```text
request.intent
request.section_source
request.sections
request.builder_sections
```

Expected `section_source` values after v0.6.3:

```text
full
profile
intent
sections
```

Expected meanings:

```text
profile
  Canonical requested profile, or empty/omitted when no profile was supplied.

intent
  Canonical explicitly requested intent, or empty/omitted when no intent was supplied.

section_source
  The highest-priority source that determined output and builder selection.

sections
  Effective canonical output sections when explicit section mode is active.

builder_sections
  Actual builder execution plan.
```

For `-Profile=ai_context` without an explicit Intent, do not invent a new user-requested Intent value. The profile may map directly to `summary,digest`, equivalent to the `quick_overview` section set, while `request.intent` remains empty or omitted according to existing serialization style.

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
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_2_IntentProfile_TaskSource.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/SectionRegistry_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ValidationPolicy_v1.md
UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/ImplementationResultLog_v1.md
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/validation_report.json
```

## Current Behavior

AssetDump v0.6.2 supports three current selection states:

```text
implicit full mode
explicit Intent-derived sections
explicit Sections
```

Current precedence:

```text
Sections > Intent > implicit full
```

Current request evidence includes:

```text
request.intent
request.section_source
request.sections
request.builder_sections
```

There is no stable named preset for callers that want summary-only, summary-plus-digest, or a default AI-oriented compact output.

## Desired Behavior

AssetDump v0.6.3 should support stable named profiles without changing existing commands.

Examples:

```text
-Profile=summary_only
  sections: summary
  builder_sections: summary

-Profile=digest_only
  sections: summary,digest
  builder_sections: summary

-Profile=ai_context
  sections: summary,digest
  builder_sections: summary

-Profile=full
  existing full output and builders
```

Higher-priority selections must override profile mappings without discarding profile metadata.

## Implementation Steps

1. Inspect v0.6.2 selection parsing and resolution in `ADumpTypes.*` and `ADumpRunOpts.*`.
2. Inspect the builder-plan handoff in `ADumpService.cpp`.
3. Inspect request metadata serialization in `ADumpJson.cpp`.
4. Inspect changed-only selection inputs in `ADumpFingerprint.cpp`.
5. Add a compact canonical profile representation using current coding style.
6. Add Korean one-line comments above new profile declarations and helpers.
7. Parse one `-Profile=` value.
8. Normalize profile names consistently with the current Intent handling style.
9. Reject empty or unknown profile values before output generation.
10. Map `summary_only`, `digest_only`, and `ai_context` to canonical section selections.
11. Preserve true full behavior for `full` without fabricating an explicit reduced section set.
12. Apply precedence in one centralized resolution path: Sections > Intent > Profile > implicit full.
13. Ensure builder planning receives only the effective resolved selection.
14. Add `request.profile` and support `section_source=profile`.
15. Preserve requested Intent and Profile metadata when a higher-priority option controls output.
16. Include profile and effective source in changed-only fingerprint inputs where needed.
17. Add automatic profile mapping smoke checks.
18. Add automatic multi-option precedence checks.
19. Add an invalid profile smoke check.
20. Preserve all existing section, builder, intent, project-batch, and changed-only checks.
21. Compile and link the AssetDump module.
22. Attempt the full project editor build and classify the existing `CFVehiclePawn.cpp` errors without modifying gameplay code if they remain the only blocker.

## Acceptance Criteria

1. No selection option preserves implicit full behavior.
2. `-Profile=full` preserves existing full output and builder behavior.
3. `-Profile=summary_only` emits `summary` and reports `builder_sections=summary`.
4. `-Profile=digest_only` emits `summary,digest` and reports `builder_sections=summary`.
5. `-Profile=ai_context` without Intent emits `summary,digest` and reports `builder_sections=summary`.
6. Profile-controlled reduced output reports `section_source=profile`.
7. `request.profile` reports the canonical requested profile.
8. Explicit Intent overrides Profile for effective output and builders.
9. Explicit Sections override both Intent and Profile for effective output and builders.
10. Lower-priority requested Profile and Intent metadata remain visible when the existing request envelope style permits it.
11. Unknown profile values fail before output generation.
12. Unknown-profile errors list all valid profile names.
13. Existing unknown Intent and unknown Section behavior remains unchanged.
14. Existing four Intent mappings remain unchanged.
15. Existing v0.6.x Plugin validation passes.
16. Profile and precedence smoke checks pass.
17. Project batch succeeds.
18. Changed-only rerun skips unchanged assets normally after any expected one-time refresh.
19. AssetDump compiles and links successfully.
20. Existing `CFVehiclePawn.cpp` errors, if still present, are reported as unrelated and are not modified in this task.
21. Existing commands require no migration.

## Verification

### AssetDump Compile and Link

Compile the editor target or another supported path that compiles and links the AssetDump module.

Required evidence:

```text
AssetDump module compile: succeeded
AssetDump DLL link: succeeded
```

Attempt:

```text
CarFight_ReEditor Win64 Development
```

If the only failure remains the two existing `CFVehiclePawn.cpp` errors:

```text
classification: unrelated_existing_issue
AssetDump feature acceptance may continue
Do not edit CFVehiclePawn.cpp
```

### Plugin Validation

```text
RunBPDumpRegression.ps1 -ValidationProfile Plugin -SkipBuild -CompactLog
```

Confirm existing Plugin fixtures and all prior v0.6.x checks pass.

### Profile Smoke Tests

```text
-Profile=full
  confirm full output compatibility
  confirm profile: full
  confirm section_source: profile

-Profile=summary_only
  confirm sections: summary
  confirm builder_sections: summary
  confirm section_source: profile

-Profile=digest_only
  confirm sections: summary,digest
  confirm builder_sections: summary
  confirm section_source: profile

-Profile=ai_context
  confirm sections: summary,digest
  confirm builder_sections: summary
  confirm section_source: profile
```

### Precedence Smoke Tests

```text
-Profile=summary_only -Intent=widget_layout
  confirm section_source: intent
  confirm sections: summary,digest,widget_designer
  confirm builder_sections: summary,widget_designer

-Profile=ai_context -Intent=widget_layout -Sections=summary,digest
  confirm section_source: sections
  confirm sections: summary,digest
  confirm builder_sections: summary
  confirm widget_designer is not emitted
```

### Invalid Profile Smoke Test

```text
-Profile=invalid_profile
  confirm exit code is non-zero
  confirm invalid_profile is shown
  confirm full, summary_only, digest_only, and ai_context are listed
```

### Integration Checks

```text
Run project batch.
Run changed-only batch again.
Confirm unchanged assets are skipped normally.
```

## Implementation Result

Status: completed.

Completion time reported:

```text
2026-07-10 14:09:36 +09:00
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

Independent verification:

```text
git diff --check: passed
static brace count check: passed
CarFight_ReEditor Win64 Development: succeeded
Plugin fixture cases: 8/8 passed
Plugin validation cases: 8/8 passed
required_failed_count: 0
selection smoke checks: 19/19 passed
/Game/CarFight project batch: 100/100 succeeded
ChangedOnly immediate rerun: 100/100 skipped
```

Profile evidence from `validation_report.json`:

```text
profile_full: passed
profile_summary_only: passed
profile_digest_only: passed
profile_ai_context: passed
profile_intent_precedence: passed
profile_intent_sections_precedence: passed
invalid_profile_rejected: passed
```

Build environment correction:

```text
reported missing path: D:/UE_5.7/Engine/Build/BatchFiles/Build.bat
resolved valid engine root: D:/UnrealEngine_Source
resolved build tool: D:/UnrealEngine_Source/Engine/Build/BatchFiles/Build.bat
```

Validation runtime caveat:

```text
UnrealEditor-Cmd process exit code: 1
known external error: HttpListener unable to bind to 127.0.0.1:8100
AssetDump reports: fresh and successful
classification: validation_environment_issue
```

The port-binding error comes from another enabled UE MCP-related plugin and does not invalidate the AssetDump reports. The report data, not the raw process exit code alone, proves the v0.6.3 feature passed. Regression harness handling for this mismatch is deferred to v0.6.4.

Validation artifacts:

```text
UE/Plugins/ue-assetdump/Dumped/BPDumpValidationPlugin/BPDumpValidationPlugin/validation_report.json
UE/Plugins/ue-assetdump/Dumped/BPDumpProjectProfileVerify/run_report.json
```

## Migration

Existing commands do not need to change.

`-Profile=` is optional. Existing implicit full mode, `-Sections=`, and `-Intent=` commands retain their behavior.

Selection precedence after v0.6.3 is:

```text
Sections > Intent > Profile > implicit full
```

Profile metadata participates in request output and changed-only fingerprinting. Existing changed-only outputs may regenerate once after the update. Subsequent unchanged runs must skip normally.

`ai_context` in v0.6.3 is only a compact selection profile. It is not the future `ai_context_bundle_v1` export.

## Unresolved

None.

## Codex Contract Source

- artifact_role: codex_task_source
- codex_input: true
- contract_version: v1.0
- source_plan: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/v0_6_3_AIContextProfile_TaskSource.md`
- output_target: `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/Generated/Final/v0_6_3_AIContextProfile_CodexTask.yaml`

## Changelog

### v1.1

- Marked v0.6.3 implementation as completed.
- Added independent build, Plugin validation, 19 selection checks, project batch, and ChangedOnly evidence.
- Documented the corrected engine root and the unrelated UE HttpListener port conflict.
- Deferred regression harness path and exit-classification hardening to v0.6.4.

### v1.0

- Promoted v0.6.3 AI Context Profile from draft to Codex-compilable TaskSource.
- Defined four profiles and the `Sections > Intent > Profile > implicit full` precedence policy.
- Defined request metadata, changed-only fingerprint behavior, build exception classification, acceptance criteria, and verification gates.
