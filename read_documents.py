#!/usr/bin/env python3
"""Extract text from PDF, DOCX, and DOC files.

Dependencies:
    pip install pypdf python-docx pywin32

Examples:
    python read_documents.py manual.pdf report.docx legacy.doc
    python read_documents.py spec.pdf -o extracted.txt
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract text from PDF, DOCX, and DOC files."
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help="One or more .pdf, .docx, or .doc files to read.",
    )
    parser.add_argument(
        "-o",
        "--output",
        help="Optional output text file. If omitted, prints to stdout.",
    )
    parser.add_argument(
        "--encoding",
        default="utf-8",
        help="Encoding used when writing the output file. Default: utf-8.",
    )
    parser.add_argument(
        "--page-separator",
        default="\n\n--- page break ---\n\n",
        help="Separator inserted between PDF pages.",
    )
    parser.add_argument(
        "--file-separator",
        default="\n\n==========\n\n",
        help="Separator inserted between multiple input files.",
    )
    return parser.parse_args()


def read_pdf(path: Path, page_separator: str) -> str:
    try:
        from pypdf import PdfReader
    except ImportError as exc:
        raise RuntimeError(
            "Missing dependency 'pypdf'. Install it with: pip install pypdf"
        ) from exc

    reader = PdfReader(str(path))
    pages: list[str] = []
    for page in reader.pages:
        pages.append((page.extract_text() or "").strip())

    return page_separator.join(pages).strip()


def read_docx(path: Path) -> str:
    try:
        from docx import Document
    except ImportError as exc:
        raise RuntimeError(
            "Missing dependency 'python-docx'. Install it with: pip install python-docx"
        ) from exc

    document = Document(str(path))
    blocks: list[str] = []

    for paragraph in document.paragraphs:
        text = paragraph.text.strip()
        if text:
            blocks.append(text)

    for table in document.tables:
        for row in table.rows:
            cells = [cell.text.strip() for cell in row.cells]
            if any(cells):
                blocks.append(" | ".join(cells))

    return "\n".join(blocks).strip()


def read_doc(path: Path) -> str:
    if sys.platform != "win32":
        raise RuntimeError(
            "Legacy .doc extraction in this script is only supported on Windows."
        )

    try:
        import pythoncom
        import win32com.client
    except ImportError as exc:
        raise RuntimeError(
            "Reading .doc requires pywin32. Install it with: pip install pywin32"
        ) from exc

    word = None
    document = None
    pythoncom.CoInitialize()
    try:
        word = win32com.client.DispatchEx("Word.Application")
        word.Visible = False
        word.DisplayAlerts = 0
        document = word.Documents.Open(str(path.resolve()), ReadOnly=True)
        return document.Content.Text.strip()
    except Exception as exc:
        raise RuntimeError(
            "Failed to read .doc via Microsoft Word automation. "
            "Make sure Microsoft Word is installed and the file is not locked. "
            f"Details: {exc}"
        ) from exc
    finally:
        if document is not None:
            document.Close(False)
        if word is not None:
            word.Quit()
        pythoncom.CoUninitialize()


def extract_text(path: Path, page_separator: str) -> str:
    suffix = path.suffix.lower()
    if suffix == ".pdf":
        return read_pdf(path, page_separator)
    if suffix == ".docx":
        return read_docx(path)
    if suffix == ".doc":
        return read_doc(path)

    raise RuntimeError(
        f"Unsupported file type: {path.suffix}. Expected .pdf, .docx, or .doc"
    )


def render_result(path: Path, text: str) -> str:
    body = text if text else "[No text extracted]"
    return f"===== {path.name} =====\n{body}".rstrip()


def main() -> int:
    args = parse_args()
    rendered_documents: list[str] = []
    had_error = False

    for raw_path in args.inputs:
        path = Path(raw_path)
        if not path.is_file():
            print(f"[ERROR] File not found: {path}", file=sys.stderr)
            had_error = True
            continue

        try:
            text = extract_text(path, args.page_separator)
        except Exception as exc:
            print(f"[ERROR] {path}: {exc}", file=sys.stderr)
            had_error = True
            continue

        rendered_documents.append(render_result(path, text))

    if not rendered_documents:
        return 1

    output_text = args.file_separator.join(rendered_documents) + "\n"

    if args.output:
        Path(args.output).write_text(output_text, encoding=args.encoding)
    else:
        sys.stdout.write(output_text)

    return 1 if had_error else 0


if __name__ == "__main__":
    raise SystemExit(main())