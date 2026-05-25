---
name: "Project Document Reading"
description: "Use when reading, searching, summarizing, or answering questions from local PDF, DOCX, DOC, datasheet, manual, specification, reference document, doc folder, or reference folder files. Covers read_documents.py, page ranges, regex matching, context lines, and output limits."
applyTo: "read_documents.py, doc/**/*, reference/**/*, **/*.pdf, **/*.docx, **/*.doc"
---

# Project Document Reading

Use these instructions when a task depends on local project documents such as PDFs, DOCX files, legacy DOC files, datasheets, vendor manuals, reference manuals, specifications, or files under `doc/` or `reference/`.

## Reader Script

- Prefer the project helper script `read_documents.py` for local PDF, DOCX, and DOC content.
- Use `& 'D:\Python\python.exe' .\read_documents.py <document-path> ...` from the repository root when the VS Code terminal cannot resolve `python` reliably.
- The script supports PDF, DOCX, and legacy DOC. PDF requires `pypdf`; DOCX works with `python-docx` when installed and otherwise falls back to basic standard-library extraction; legacy DOC requires Windows, Microsoft Word, and `pywin32`.
- Write large or reusable extraction output under `build/doc-cache/`, which is ignored by Git in this project.

## Matching Rules

- Start with a narrow query instead of extracting an entire large document.
- For PDF files, use `--pages <range>` when the relevant pages are known, for example `--pages 76-78` or `--pages 1,3,10-12`.
- For PDF, DOCX, or DOC files, use `--grep <pattern> --context <lines> --max-chars <limit>` to extract matching snippets.
- Treat `--grep` as a regular expression. Matching is case-insensitive by default; add `--case-sensitive` only when exact case matters.
- Quote grep patterns in PowerShell. Use alternation for related terms, such as `--grep "HRTIM|ADC trigger|fault input"`.
- Repeat `--grep` for multiple alternative patterns; a line is kept when it matches any pattern.
- Use `--context 3` to `--context 10` for most questions. Increase context only when the extracted snippet is too narrow.
- Use `--max-chars` for chat-sized output, commonly `12000` to `30000`, and lower it for quick checks.
- If the script returns `[No matches]`, broaden the pattern, try synonyms, inspect the table of contents, or search a wider page range.
- If the script returns `[No text extracted]` for a PDF, the document may be scanned or image-only and may need OCR before this script can read it.

## Answering From Documents

- Base document answers on extracted text, not memory alone.
- Mention the document name and page or section markers when they materially support the answer.
- For vendor reference manuals, distinguish between a family-wide superset and the exact part/package availability described in datasheets or project configuration files.
- Prefer concise excerpts and synthesis over dumping large document blocks into the response.
