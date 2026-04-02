import os
import subprocess
from datetime import datetime
from pathlib import Path

import yaml

def extract_title(path: Path) -> str:
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("# "):
            return line[2:].strip()
    return path.stem


def should_skip_doc(rel_path: str) -> bool:
    return any(part.startswith(".") for part in Path(rel_path).parts)


def build_page_url(rel_path: str) -> str:
    if rel_path.endswith("/index.md"):
        return rel_path[: -len("index.md")]
    if rel_path == "index.md":
        return ""
    return rel_path.removesuffix(".md") + "/"


def is_content_page(rel_path: str) -> bool:
    return rel_path != "index.md" and not rel_path.endswith("/index.md")


def get_git_commit_timestamp(repo_root: Path, rel_path: str) -> float | None:
    result = subprocess.run(
        ["git", "-C", str(repo_root), "log", "-1", "--format=%ct", "--", f"docs/{rel_path}"],
        capture_output=True,
        text=True,
        check=False,
    )
    value = result.stdout.strip()
    if result.returncode != 0 or not value:
        return None
    try:
        return float(value)
    except ValueError:
        return None


def get_git_first_commit_timestamp(repo_root: Path, rel_path: str) -> float | None:
    result = subprocess.run(
        [
            "git",
            "-C",
            str(repo_root),
            "log",
            "--diff-filter=A",
            "--follow",
            "--format=%ct",
            "--",
            f"docs/{rel_path}",
        ],
        capture_output=True,
        text=True,
        check=False,
    )
    values = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    if result.returncode != 0 or not values:
        return None
    try:
        return float(values[0])
    except ValueError:
        return None


def collect_markdown_pages(docs_dir: Path) -> list[dict]:
    repo_root = docs_dir.parent
    pages = []
    for path in sorted(docs_dir.rglob("*.md")):
        rel_path = path.relative_to(docs_dir).as_posix()
        if should_skip_doc(rel_path):
            continue

        parts = rel_path.split("/", 1)
        section = parts[0] if len(parts) > 1 else "home"
        git_created_ts = get_git_first_commit_timestamp(repo_root, rel_path)
        git_updated_ts = get_git_commit_timestamp(repo_root, rel_path)
        updated_ts = git_updated_ts or path.stat().st_mtime
        updated_dt = datetime.fromtimestamp(updated_ts)

        pages.append(
            {
                "path": rel_path,
                "title": extract_title(path),
                "section": section,
                "section_label": section.capitalize(),
                "git_created_ts": git_created_ts,
                "git_updated_ts": git_updated_ts,
                "updated_ts": updated_ts,
                "updated_label": updated_dt.strftime("%Y-%m-%d"),
                "url": build_page_url(rel_path),
            }
        )
    return pages


def build_homepage_stats(pages: list[dict]) -> dict:
    content_pages = [page for page in pages if is_content_page(page["path"])]
    committed_pages = [page for page in content_pages if page["git_updated_ts"] is not None]
    latest_page = max(committed_pages, key=lambda page: page["git_updated_ts"], default=None)
    content_sections = {page["section"] for page in content_pages if page["section"] != "home"}
    recent_threshold = datetime.now().timestamp() - 30 * 24 * 60 * 60

    return {
        "document_count": len(content_pages),
        "section_count": len(content_sections),
        "code_count": sum(1 for page in content_pages if page["section"] == "code"),
        "latest_update": latest_page["updated_label"] if latest_page else "-",
        "recent_30d_count": sum(
            1 for page in committed_pages if page["git_updated_ts"] >= recent_threshold
        ),
        "recent_30d_added_count": sum(
            1
            for page in committed_pages
            if page["git_created_ts"] is not None and page["git_created_ts"] >= recent_threshold
        ),
    }


def build_recent_updates(pages: list[dict], limit: int = 6) -> list[dict]:
    content_pages = [
        page
        for page in pages
        if is_content_page(page["path"]) and page["git_updated_ts"] is not None
    ]
    ranked = sorted(content_pages, key=lambda page: page["git_updated_ts"], reverse=True)
    return ranked[:limit]


def load_homepage_sections(config_path: Path) -> list[dict]:
    data = yaml.safe_load(config_path.read_text(encoding="utf-8")) or {}
    sections = data.get("sections", [])
    enabled = [section for section in sections if section.get("enabled", True)]
    return sorted(enabled, key=lambda section: section.get("order", 999))


def define_env(env):
    """
    This is the hook for the variables, macros and filters.
    """

    @env.macro
    def doc_env():
        "Document the environment"
        return {name: getattr(env, name) for name in dir(env) if not name.startswith("_")}

    @env.macro
    def include_file(filename, start_line=0, end_line=None):
        """
        Include a file, optionally indicating start_line and end_line
        (start counting from 0)
        The path is relative to the top directory of the documentation
        project.
        """
        full_filename = os.path.join(env.project_dir, filename)
        with open(full_filename, "r") as f:
            lines = f.readlines()
        line_range = lines[start_line:end_line]
        return "".join(line_range)

    @env.macro
    def homepage_stats():
        docs_dir = Path(env.project_dir) / "docs"
        return build_homepage_stats(collect_markdown_pages(docs_dir))

    @env.macro
    def homepage_recent_updates(limit=6):
        docs_dir = Path(env.project_dir) / "docs"
        return build_recent_updates(collect_markdown_pages(docs_dir), limit=limit)

    @env.macro
    def homepage_sections():
        config_path = Path(env.project_dir) / "data" / "homepage_sections.yml"
        return load_homepage_sections(config_path)
