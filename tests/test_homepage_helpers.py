import tempfile
import unittest
from datetime import datetime, timedelta
from pathlib import Path
from unittest.mock import patch

from main import (
    build_homepage_stats,
    build_recent_updates,
    collect_markdown_pages,
    get_git_first_commit_timestamp,
    get_git_commit_timestamp,
    load_homepage_sections,
)


class HomepageHelpersTest(unittest.TestCase):
    def test_collect_markdown_pages_skips_hidden_paths(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "docs").mkdir()
            (root / "docs" / "index.md").write_text("# Home\n", encoding="utf-8")
            (root / "docs" / ".meta.yml").write_text("x: 1\n", encoding="utf-8")
            (root / "docs" / "blog").mkdir()
            (root / "docs" / "blog" / "index.md").write_text("# Blog\n", encoding="utf-8")
            (root / "docs" / ".internal").mkdir(parents=True, exist_ok=True)
            (root / "docs" / ".internal" / "plan.md").write_text("# Hidden\n", encoding="utf-8")

            pages = collect_markdown_pages(root / "docs")
            rels = [page["path"] for page in pages]

            self.assertIn("index.md", rels)
            self.assertIn("blog/index.md", rels)
            self.assertNotIn(".internal/plan.md", rels)

    def test_build_homepage_stats_counts_docs_and_sections(self):
        now = datetime.now()
        pages = [
            {"path": "blog/post-a.md", "title": "Post A", "section": "blog", "git_created_ts": (now - timedelta(days=1)).timestamp(), "git_updated_ts": (now - timedelta(days=1)).timestamp(), "updated_ts": 5, "updated_label": "2026-04-05"},
            {"path": "blog/index.md", "title": "Blog", "section": "blog", "git_created_ts": (now - timedelta(days=7)).timestamp(), "git_updated_ts": (now - timedelta(days=7)).timestamp(), "updated_ts": 3, "updated_label": "2026-04-01"},
            {"path": "code/topic.md", "title": "Code Topic", "section": "code", "git_created_ts": (now - timedelta(days=35)).timestamp(), "git_updated_ts": (now - timedelta(days=10)).timestamp(), "updated_ts": 4, "updated_label": "2026-04-04"},
            {"path": "learning/index.md", "title": "Learning", "section": "learning", "git_created_ts": (now - timedelta(days=20)).timestamp(), "git_updated_ts": (now - timedelta(days=20)).timestamp(), "updated_ts": 2, "updated_label": "2026-03-31"},
            {"path": "learning/cuda/Basic.md", "title": "CUDA Basic", "section": "learning", "git_created_ts": (now - timedelta(days=40)).timestamp(), "git_updated_ts": (now - timedelta(days=40)).timestamp(), "updated_ts": 1, "updated_label": "2026-03-30"},
            {"path": "interview/systemdesign/question.md", "title": "System Design", "section": "interview", "git_created_ts": (now - timedelta(days=15)).timestamp(), "git_updated_ts": (now - timedelta(days=20)).timestamp(), "updated_ts": 2.5, "updated_label": "2026-04-02"},
        ]

        stats = build_homepage_stats(pages)

        self.assertEqual(stats["document_count"], 4)
        self.assertEqual(stats["section_count"], 4)
        self.assertEqual(stats["code_count"], 1)
        self.assertEqual(stats["latest_update"], "2026-04-05")
        self.assertEqual(stats["recent_30d_count"], 3)
        self.assertEqual(stats["recent_30d_added_count"], 2)

    def test_build_recent_updates_sorts_desc_and_limits_results(self):
        pages = [
            {"path": "a.md", "title": "A", "section": "code", "git_created_ts": 1, "git_updated_ts": 1, "updated_ts": 1, "updated_label": "2026-03-30"},
            {"path": "blog/index.md", "title": "Blog", "section": "blog", "git_created_ts": 3, "git_updated_ts": 3, "updated_ts": 3, "updated_label": "2026-04-01"},
            {"path": "c.md", "title": "C", "section": "learning", "git_created_ts": 2, "git_updated_ts": 2, "updated_ts": 2, "updated_label": "2026-03-31"},
            {"path": "draft.md", "title": "Draft", "section": "learning", "git_created_ts": None, "git_updated_ts": None, "updated_ts": 8, "updated_label": "2026-04-08"},
        ]

        updates = build_recent_updates(pages, limit=2)

        self.assertEqual([item["title"] for item in updates], ["C", "A"])

    def test_build_homepage_stats_ignores_uncommitted_for_latest_update(self):
        pages = [
            {"path": "code/a.md", "title": "A", "section": "code", "git_created_ts": 4, "git_updated_ts": 4, "updated_ts": 4, "updated_label": "2026-04-04"},
            {"path": "learning/draft.md", "title": "Draft", "section": "learning", "git_created_ts": None, "git_updated_ts": None, "updated_ts": 9, "updated_label": "2026-04-09"},
        ]

        stats = build_homepage_stats(pages)

        self.assertEqual(stats["latest_update"], "2026-04-04")

    def test_load_homepage_sections_returns_enabled_items_in_order(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "data").mkdir()
            (root / "data" / "homepage_sections.yml").write_text(
                """
sections:
  - key: learning
    title: 学习笔记
    eyebrow: Learning
    description: 学习中的长期记录
    url: learning/
    order: 2
    enabled: true
  - key: hidden
    title: Hidden
    eyebrow: Hidden
    description: should not render
    url: hidden/
    order: 1
    enabled: false
  - key: blog
    title: 博客
    eyebrow: Blog
    description: 技术随笔
    url: blog/
    order: 1
    enabled: true
""".strip(),
                encoding="utf-8",
            )

            sections = load_homepage_sections(root / "data" / "homepage_sections.yml")

            self.assertEqual([section["key"] for section in sections], ["blog", "learning"])

    @patch("main.subprocess.run")
    def test_get_git_commit_timestamp_returns_commit_time(self, mock_run):
        mock_run.return_value.returncode = 0
        mock_run.return_value.stdout = "1712400000\n"

        ts = get_git_commit_timestamp(Path("/repo"), "learning/cuda/Basic.md")

        self.assertEqual(ts, 1712400000.0)

    @patch("main.subprocess.run")
    def test_get_git_commit_timestamp_returns_none_when_untracked(self, mock_run):
        mock_run.return_value.returncode = 0
        mock_run.return_value.stdout = "\n"

        ts = get_git_commit_timestamp(Path("/repo"), "learning/cuda/Basic.md")

        self.assertIsNone(ts)

    @patch("main.subprocess.run")
    def test_get_git_first_commit_timestamp_returns_commit_time(self, mock_run):
        mock_run.return_value.returncode = 0
        mock_run.return_value.stdout = "1711000000\n"

        ts = get_git_first_commit_timestamp(Path("/repo"), "learning/cuda/Basic.md")

        self.assertEqual(ts, 1711000000.0)


if __name__ == "__main__":
    unittest.main()
