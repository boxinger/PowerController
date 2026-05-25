---
name: template-skill
description: 'Template skill for creating a custom Copilot workflow. Use when drafting a new skill, defining steps, adding references, or packaging reusable project guidance.'
argument-hint: 'Describe the workflow or domain to specialize'
user-invocable: true
---

# Template Skill

Use this starter skill as a copyable base for a real workflow-specific skill.

Before using it in production:

1. Rename the folder to your real skill name.
2. Update the `name` field so it exactly matches the folder name.
3. Replace the generic sections below with domain-specific instructions.
4. Add referenced files under `./references/`, `./scripts/`, or `./assets/` only when needed.

## What This Skill Does

Describe the job this skill performs in one short paragraph.

Example:

- Create repeatable implementation plans
- Run a multi-step review workflow
- Generate files from templates
- Apply a domain-specific procedure

## When to Use

List the exact triggers the agent should associate with this skill.

Example triggers:

- The user asks to perform a repeatable workflow
- The task needs a fixed sequence of steps
- The task benefits from bundled references or templates
- The same domain knowledge is reused across many chats

## Inputs

Document what the user should provide.

Example inputs:

- Target files or folders
- The desired output format
- Constraints, conventions, or acceptance criteria
- Optional flags such as quick, standard, or thorough

## Procedure

1. Identify the concrete target and collect only the minimum required context.
2. Confirm the controlling file, symbol, or workflow entry point.
3. Apply the domain-specific procedure for this skill.
4. Produce the requested output or make the required changes.
5. Run the narrowest useful validation available.
6. Report the result, including any assumptions or remaining risks.

## Output

Define what success looks like.

Example outputs:

- A code change with validation
- A structured review with findings first
- A generated document or boilerplate file
- A summarized decision with clear next steps

## Optional Resources

Add resources only if they improve reuse or keep this file short.

- `./references/` for longer docs or checklists
- `./scripts/` for helper scripts
- `./assets/` for templates, snippets, or sample outputs

This template includes a generic, project-neutral example layout:

- `./references/example-checklist.md` shows how to store a longer checklist
- `./assets/example-output-template.md` shows how to store an output template
- `./scripts/README.md` explains how script helpers should be documented

When adapting this skill, keep `SKILL.md` as the entry point and link only the resources that are useful for the workflow.

## Author Checklist

- Folder name matches the `name` field exactly
- `description` includes strong trigger keywords
- Instructions are specific and procedural
- Extra resources are linked with relative `./` paths
- The skill stays short and loads details progressively