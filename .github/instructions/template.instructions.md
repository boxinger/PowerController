---
name: "Template Instructions"
description: "Use when creating a project-neutral instruction file, defining reusable coding preferences, documenting file-matching rules, or drafting long-lived AI guidance."
applyTo: "docs/**/*.md"
---

# Template Instructions

Use this file as a copyable base for a real `.instructions.md` file.

Before using it in production:

1. Rename the file to match the instruction topic.
2. Update `name` to a clear display name.
3. Update `description` with specific trigger keywords.
4. Update or remove `applyTo` based on the files this instruction should affect.
5. Replace the generic sections below with concise, actionable guidance.

## Purpose

Describe the long-lived rule, preference, or constraint this instruction provides.

Good examples:

- Coding style conventions
- Review output expectations
- Documentation writing preferences
- API design constraints
- Test authoring rules

## When To Apply

Use this instruction when the task involves the topic described above, or when files match the `applyTo` pattern.

Keep this section focused. If the guidance only applies to a specific workflow, consider using a skill instead.

## Rules

- Keep each rule specific and actionable.
- Prefer short bullets over long explanations.
- Avoid mixing unrelated concerns in the same instruction file.
- Avoid rules that conflict with other instruction files.
- Put detailed background information in separate documentation and link to it when needed.

## Examples

Use examples only when they clarify the rule.

Good:

```text
Prefer small, focused changes over broad refactors unless the task requires a redesign.
```

Avoid:

```text
Write good code.
```

## Maintenance Checklist

- `description` starts with a clear "Use when" style trigger.
- `applyTo` is narrow enough to avoid unnecessary context loading.
- The file covers one concern only.
- The rules are stable enough to apply across many future tasks.
- Any project-specific details are intentional.